<#
.SYNOPSIS
    Windows (PowerShell) equivalent of install.sh -- checks for (and offers
    to install) everything tdf.py needs.

.DESCRIPTION
    Checks for python3 (runs tdf.py itself), arm-none-eabi-gcc, cmake
    (3.20+), ninja, and a private venv at tdf_venv\ with pyserial in it
    (only used by `tdf.py monitor`). OpenOCD is not checked here -- it's
    bundled in Generation\AUDK32_A34xxxx-1.0.12\Tool\OpenOCD\ for
    Linux/macOS/Windows and needs no separate install.

    pyserial goes in a dedicated venv rather than a plain `pip install
    pyserial` so this repo's one Python dependency stays out of your system
    Python entirely, and so it works the same way the macOS/Linux install.sh
    sets it up. `. .\export.ps1` activates it automatically.

    Safe by design: this script only ever *offers* an install command and
    asks before running it (or -Yes to skip asking) -- it never silently
    changes your system. Re-run any time; already-installed tools are just
    reported and skipped.

    For missing tools (Python included) it prefers winget (ships with
    Windows 10 1809+ / Windows 11), then Chocolatey (choco) if that's what
    you have. A freshly installed tool is not on PATH in the current
    window, so after an install this script tells you to open a new
    terminal and re-run it to finish.

.PARAMETER Yes
    Don't ask for confirmation before installing missing tools.

.EXAMPLE
    .\install.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\install.ps1 -Yes

.NOTES
    If PowerShell refuses to run this ("running scripts is disabled on this
    system"), either launch it as shown in the second example above, or run
    once:  Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
#>

[CmdletBinding()]
param(
    [Alias('y')]
    [switch]$Yes
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
# Native commands (winget, choco, python, ...) here signal failure only via
# exit code -- we check $LASTEXITCODE where it matters. Keep a non-zero exit
# from throwing (PowerShell 7.4+ makes that the default); harmless no-op on
# Windows PowerShell 5.1.
$PSNativeCommandUseErrorActionPreference = $false

$RepoRoot = $PSScriptRoot
$VenvDir  = Join-Path $RepoRoot 'tdf_venv'

# --- helpers --------------------------------------------------------------

function Test-Cmd([string]$name) {
    $null -ne (Get-Command $name -ErrorAction SilentlyContinue)
}

function Get-ToolVersion([string]$name) {
    try {
        $out = & $name --version 2>&1 | Select-Object -First 1
        return "$out".Trim()
    } catch {
        return ''
    }
}

# The venv's own python.exe path, or $null if the venv doesn't exist yet.
# Windows-native venvs use Scripts\; a venv made from a POSIX Python (e.g.
# under Git Bash calling a MSYS python) would use bin\ -- check both.
function Get-VenvPython {
    $win = Join-Path $VenvDir 'Scripts\python.exe'
    if (Test-Path $win) { return $win }
    $posix = Join-Path $VenvDir 'bin\python3'
    if (Test-Path $posix) { return $posix }
    return $null
}

# A *working* system Python 3, as an argv array: the py launcher
# (python.org default) then python.exe. The bare "python.exe" App Execution
# Alias that only opens the Microsoft Store is filtered out -- it prints
# nothing and exits non-zero for --version. Returns nothing when not found;
# always capture the call as @(Get-SystemPython) so a 1-element result
# doesn't unroll to a bare string.
function Get-SystemPython {
    if (Test-Cmd 'py') {
        try {
            $v = & py -3 --version 2>&1
            if ($LASTEXITCODE -eq 0 -and "$v" -match 'Python 3') { return @('py', '-3') }
        } catch { }
    }
    if (Test-Cmd 'python') {
        try {
            $v = & python --version 2>&1
            if ($LASTEXITCODE -eq 0 -and "$v" -match 'Python 3') { return @('python') }
        } catch { }
    }
    return
}

function Confirm-Run([string]$desc) {
    if ($Yes) { return $true }
    $reply = Read-Host "Run: $desc ? [y/N]"
    if ($reply -match '^(y|yes)$') { return $true }
    Write-Host '  skipped'
    return $false
}

# Pick a package manager. winget first, then choco.
function Get-PackageManager {
    if (Test-Cmd 'winget') { return 'winget' }
    if (Test-Cmd 'choco')  { return 'choco' }
    return $null
}

function Install-WithManager([string]$mgr, [string]$tool) {
    switch ($tool) {
        'python' {
            if ($mgr -eq 'winget') {
                Write-Host 'python3: winget catalog ids are minor-version pinned; using 3.13.'
                Write-Host '         Pick a newer Python.Python.3.x by hand if you want one.'
                if (Confirm-Run 'winget install Python.Python.3.13') {
                    winget install --id Python.Python.3.13 -e --accept-package-agreements --accept-source-agreements
                }
            } else {
                if (Confirm-Run 'choco install python') { choco install python -y }
            }
        }
        'cmake' {
            if ($mgr -eq 'winget') {
                if (Confirm-Run 'winget install Kitware.CMake') {
                    winget install --id Kitware.CMake -e --accept-package-agreements --accept-source-agreements
                }
            } else {
                if (Confirm-Run 'choco install cmake') { choco install cmake -y }
            }
        }
        'ninja' {
            if ($mgr -eq 'winget') {
                if (Confirm-Run 'winget install Ninja-build.Ninja') {
                    winget install --id Ninja-build.Ninja -e --accept-package-agreements --accept-source-agreements
                }
            } else {
                if (Confirm-Run 'choco install ninja') { choco install ninja -y }
            }
        }
        'arm-none-eabi-gcc' {
            Write-Host ''
            Write-Host 'arm-none-eabi-gcc: the package id for the Arm GNU bare-metal toolchain'
            Write-Host 'has changed names over the years, so this script will not guess. Options:'
            if ($mgr -eq 'winget') {
                Write-Host '  1) winget install Arm.GnuArmEmbeddedToolchain   (older id, may still work)'
            } else {
                Write-Host '  1) choco install gcc-arm-embedded'
            }
            Write-Host '  2) Official installer from Arm: search "Arm GNU Toolchain Downloads" on'
            Write-Host '     developer.arm.com and pick the AArch32 bare-metal (arm-none-eabi)'
            Write-Host '     build for Windows. Tick "Add path to environment variable" in the'
            Write-Host '     installer, then open a new terminal.'
            if ($mgr -eq 'winget') {
                if (Confirm-Run 'winget install Arm.GnuArmEmbeddedToolchain (option 1)') {
                    winget install --id Arm.GnuArmEmbeddedToolchain -e --accept-package-agreements --accept-source-agreements
                }
            } else {
                if (Confirm-Run 'choco install gcc-arm-embedded (option 1)') {
                    choco install gcc-arm-embedded -y
                }
            }
        }
    }
}

# --- check tools --------------------------------------------------------

$Missing = @()

Write-Host 'Checking for required tools...'

$SysPy = @(Get-SystemPython)   # @() not found, @('python') or @('py','-3') otherwise
if ($SysPy.Count -gt 0) {
    $PyExe  = $SysPy[0]
    $PyRest = @($SysPy | Select-Object -Skip 1)   # @('-3') for the launcher, @() for python.exe
    $pyVer  = "$(& $PyExe @PyRest --version 2>&1)".Trim()
    Write-Host "  [x] python3 -- $pyVer"
} else {
    Write-Host '  [ ] python3 -- not found (runs tdf.py itself)'
    $Missing += 'python'
}

foreach ($tool in @('arm-none-eabi-gcc', 'cmake', 'ninja')) {
    if (Test-Cmd $tool) {
        Write-Host "  [x] $tool -- $(Get-ToolVersion $tool)"
    } else {
        Write-Host "  [ ] $tool -- not found"
        $Missing += $tool
    }
}

# --- check pyserial in the venv --------------------------------------

$VenvPython  = Get-VenvPython
$PyserialOk  = $false
if ($VenvPython) {
    try {
        & $VenvPython -c 'import serial' 2>$null
        if ($LASTEXITCODE -eq 0) {
            $PyserialOk  = $true
            $PyserialVer = (& $VenvPython -c 'import serial; print(serial.__version__)' 2>$null)
            Write-Host "  [x] pyserial -- $("$PyserialVer".Trim()), in tdf_venv\ (used by: tdf.py monitor)"
        }
    } catch { }
}
if (-not $PyserialOk) {
    Write-Host '  [ ] pyserial -- not found in tdf_venv\ (used by: tdf.py monitor)'
}

# --- install missing build tools / python3 --------------------------------

if ($Missing.Count -gt 0) {
    Write-Host ''
    Write-Host "Missing: $($Missing -join ', ')"

    $mgr = Get-PackageManager
    if (-not $mgr) {
        Write-Host ''
        Write-Host 'No winget or choco found. Install these manually, then re-run .\install.ps1:'
        if ($Missing -contains 'python') {
            Write-Host '  python3:          https://www.python.org/downloads/  (tick "Add python.exe to PATH")'
        }
        if ($Missing -contains 'cmake') {
            Write-Host '  cmake:            https://cmake.org/download/  (tick "Add CMake to the system PATH")'
        }
        if ($Missing -contains 'ninja') {
            Write-Host '  ninja:            https://github.com/ninja-build/ninja/releases  (put ninja.exe on PATH)'
        }
        if ($Missing -contains 'arm-none-eabi-gcc') {
            Write-Host '  arm-none-eabi-gcc: "Arm GNU Toolchain Downloads" on developer.arm.com,'
            Write-Host '                    run the Windows installer, tick "Add path to environment variable".'
        }
        exit 1
    }

    Write-Host "Using package manager: $mgr"
    foreach ($tool in $Missing) {
        Install-WithManager $mgr $tool
    }

    Write-Host ''
    Write-Host 'Open a NEW terminal (so PATH changes take effect), then re-run .\install.ps1'
    Write-Host 'to finish -- it sets up tdf_venv\ + pyserial once python3 is visible.'
    exit 0
}

# --- venv + pyserial (a working system python3 is present here) ----------

if (-not $PyserialOk) {
    Write-Host ''
    $doVenv = [bool]$Yes
    if (-not $Yes) {
        $reply = Read-Host 'Create a venv at tdf_venv\ and install pyserial into it? [y/N]'
        $doVenv = ($reply -match '^(y|yes)$')
        if (-not $doVenv) { Write-Host '  skipped' }
    }

    if ($doVenv) {
        $PyExe  = $SysPy[0]
        $PyRest = @($SysPy | Select-Object -Skip 1)   # $SysPy has >=1 element here
        if (-not (Get-VenvPython)) {
            Write-Host "  creating $VenvDir"
            & $PyExe @PyRest -m venv $VenvDir
        }
        $VenvPython = Get-VenvPython
        if (-not $VenvPython) {
            Write-Warning "venv creation appears to have failed -- 'tdf.py monitor' won't work."
        } else {
            & $VenvPython -m pip install --upgrade pip
            & $VenvPython -m pip install pyserial
            if ($LASTEXITCODE -eq 0) { $PyserialOk = $true }
        }
    }
}

# --- done ------------------------------------------------------------------

Write-Host ''
if ($PyserialOk) {
    Write-Host 'All required tools are installed.'
} else {
    Write-Host 'Build/flash tools are present; pyserial (tdf_venv\) is not, so'
    Write-Host "'tdf.py monitor' won't work yet. Re-run .\install.ps1 to set it up."
}
Write-Host ''
Write-Host 'Next:  . .\export.ps1   then   cd Getting_Started\01-Hello_Blink\Sw   then   tdf.py flash monitor'
exit 0
