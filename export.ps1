<#
.SYNOPSIS
    Windows (PowerShell) equivalent of export.sh -- adds tools\ to PATH so
    `tdf.py` can be run bare from any directory in this shell session,
    activates tdf_venv\ so `tdf.py monitor` can find pyserial, and installs
    tab completion for the tdf.py command names. Mirrors ESP-IDF's
    export.ps1, venv activation included.

.DESCRIPTION
    The PATH addition is a convenience, not a requirement: `python
    <repo>\tools\tdf.py build` works identically without ever running this
    script, since tdf.py locates the repo itself from its own file path.
    `monitor` specifically does need pyserial on the interpreter's path,
    though -- either dot-source this script first, or run tdf.py with
    tdf_venv's own python.exe directly.

    Must be DOT-SOURCED, not executed -- it edits the CURRENT shell's
    session state (PATH, the `tdf` function, the argument completer):

        . .\export.ps1

    Running it as  .\export.ps1  instead only edits a child scope that
    disappears the moment the script returns.

.NOTES
    If PowerShell refuses to run this ("running scripts is disabled"), run
    once:  Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
#>

# Best-effort dot-source check. When dot-sourced, $MyInvocation.InvocationName
# is '.'; when executed, it's the script path/name. Not every host reports
# this identically, so this is a warning, not a hard stop.
if ($MyInvocation.InvocationName -ne '.' -and $MyInvocation.InvocationName -ne '') {
    Write-Warning 'export.ps1 looks like it was executed, not dot-sourced. Run it as:  . .\export.ps1'
    Write-Warning 'PATH / venv / completion changes made below will not survive otherwise.'
}

$TdfPath = $PSScriptRoot
$env:TDF_PATH = $TdfPath
$toolsDir = Join-Path $TdfPath 'tools'

# Prepend tools\ to PATH for this session (idempotent -- re-sourcing is a no-op).
$pathParts = $env:PATH -split ';'
if ($pathParts -notcontains $toolsDir) {
    $env:PATH = "$toolsDir;$env:PATH"
}

Write-Host 'Tiremo Development Framework: tools\ added to PATH (tdf.py / tdf).'

# A `tdf` function so `tdf build` works from PowerShell even without .PY on
# PATHEXT. Prefers the venv interpreter (pyserial for `monitor`) when present.
function global:tdf {
    $env:TDF_PROG = 'tdf'
    $script = Join-Path $env:TDF_PATH 'tools\tdf.py'
    $venvPy = Join-Path $env:TDF_PATH 'tdf_venv\Scripts\python.exe'
    if (Test-Path $venvPy) {
        & $venvPy $script @args
    } elseif (Get-Command py -ErrorAction SilentlyContinue) {
        & py -3 $script @args
    } else {
        & python $script @args
    }
}

# Tab-complete tdf.py / tdf command names (build / flash / monitor / ...).
$completion = Join-Path $toolsDir 'completions\tdf.ps1'
if (Test-Path $completion) {
    . $completion
    Write-Host 'tdf.py tab completion enabled (PowerShell).'
}

# Activate tdf_venv\ (created by install.ps1) so `python` / `pip` -- and the
# `tdf` function above -- resolve to the venv with pyserial in it.
# VIRTUAL_ENV_DISABLE_PROMPT keeps this from rewriting a customized prompt.
$activate = Join-Path $TdfPath 'tdf_venv\Scripts\Activate.ps1'
$activatePosix = Join-Path $TdfPath 'tdf_venv\bin\Activate.ps1'
if (Test-Path $activate) {
    $env:VIRTUAL_ENV_DISABLE_PROMPT = '1'
    . $activate
    Write-Host "tdf_venv activated (pyserial available for 'tdf.py monitor')."
} elseif (Test-Path $activatePosix) {
    $env:VIRTUAL_ENV_DISABLE_PROMPT = '1'
    . $activatePosix
    Write-Host "tdf_venv activated (pyserial available for 'tdf.py monitor')."
} else {
    Write-Host "No tdf_venv\ found -- 'tdf.py monitor' needs pyserial. Run .\install.ps1 once to set it up."
}

Write-Host 'Try: cd Getting_Started\01-Hello_Blink\Sw ; tdf flash monitor'
