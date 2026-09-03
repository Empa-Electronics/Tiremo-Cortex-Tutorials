@echo off
setlocal enabledelayedexpansion

rem Windows (cmd.exe) equivalent of install.sh -- checks for (and offers to
rem install) everything tdf.py needs: python3 (runs tdf.py itself),
rem arm-none-eabi-gcc, cmake (3.20+), ninja, and a private venv at
rem tdf_venv\ with pyserial in it (only used by `tdf.py monitor`). OpenOCD
rem is bundled in the vendor SDK and needs no separate install.
rem
rem See install.ps1 for the PowerShell version with nicer prompts; this is
rem the plain-batch path for cmd.exe. Only ever *offers* an install command
rem and asks first (or -y to skip asking). A freshly installed tool is not
rem on PATH in this window, so after an install this tells you to open a
rem new terminal and re-run. Re-run any time.
rem
rem Usage: install.bat [-y]

set "ASSUME_YES=0"
for %%a in (%*) do (
    if /i "%%a"=="-y"    set "ASSUME_YES=1"
    if /i "%%a"=="/y"    set "ASSUME_YES=1"
    if /i "%%a"=="--yes" set "ASSUME_YES=1"
)

set "REPO_ROOT=%~dp0"
if "%REPO_ROOT:~-1%"=="\" set "REPO_ROOT=%REPO_ROOT:~0,-1%"
set "VENV_DIR=%REPO_ROOT%\tdf_venv"
set "VENV_PY=%VENV_DIR%\Scripts\python.exe"

set "MISSING="

echo Checking for required tools...
call :check_python
call :check_tool arm-none-eabi-gcc
call :check_tool cmake
call :check_tool ninja

set "PYSERIAL_OK=0"
if exist "%VENV_PY%" (
    "%VENV_PY%" -c "import serial" >nul 2>&1 && set "PYSERIAL_OK=1"
)
if "%PYSERIAL_OK%"=="1" (
    echo   [x] pyserial -- in tdf_venv\ ^(used by: tdf.py monitor^)
) else (
    echo   [ ] pyserial -- not found in tdf_venv\ ^(used by: tdf.py monitor^)
)

rem ---- install missing build tools / python3 (needs winget or choco) ----
if not defined MISSING goto :venv_setup

echo.
echo Missing:%MISSING%

set "MGR="
where winget >nul 2>&1 && set "MGR=winget"
if not defined MGR ( where choco >nul 2>&1 && set "MGR=choco" )

if not defined MGR (
    echo.
    echo No winget or choco found. Install the missing tools manually, then re-run:
    echo   python3:           https://www.python.org/downloads/  ^(tick "Add python.exe to PATH"^)
    echo   cmake:             https://cmake.org/download/  ^(tick "Add CMake to the system PATH"^)
    echo   ninja:             https://github.com/ninja-build/ninja/releases  ^(put ninja.exe on PATH^)
    echo   arm-none-eabi-gcc: search "Arm GNU Toolchain Downloads" on developer.arm.com,
    echo                      run the Windows installer, tick "Add path to environment variable".
    exit /b 1
)

echo Using package manager: %MGR%
for %%t in (%MISSING%) do call :install_tool %%t

echo.
echo Open a NEW terminal ^(so PATH changes take effect^), then re-run install.bat
echo to finish -- it sets up tdf_venv\ + pyserial once python3 is visible.
exit /b 0

rem ---- venv + pyserial (python3 confirmed present above) ----
:venv_setup
if "%PYSERIAL_OK%"=="1" goto :final

echo.
set "DO_VENV=%ASSUME_YES%"
if "%ASSUME_YES%"=="0" (
    set /p "reply=Create a venv at tdf_venv\ and install pyserial into it? [y/N] "
    if /i "!reply!"=="y"   set "DO_VENV=1"
    if /i "!reply!"=="yes" set "DO_VENV=1"
    if not "!DO_VENV!"=="1" echo   skipped
)
if not "!DO_VENV!"=="1" goto :final

call :find_python
if not defined PYTHON (
    echo warning: python3 not on PATH -- open a new terminal and re-run install.bat
    echo          to create tdf_venv\ + install pyserial.
    goto :final
)
if not exist "%VENV_PY%" (
    echo   creating %VENV_DIR%
    !PYTHON! -m venv "%VENV_DIR%"
)
if exist "%VENV_PY%" (
    "%VENV_PY%" -m pip install --upgrade pip
    "%VENV_PY%" -m pip install pyserial && set "PYSERIAL_OK=1"
) else (
    echo warning: venv creation appears to have failed -- 'tdf.py monitor' won't work.
)

:final
echo.
if "%PYSERIAL_OK%"=="1" (
    echo All required tools are installed.
) else (
    echo Build/flash tools are present; pyserial ^(tdf_venv\^) is not, so
    echo 'tdf.py monitor' won't work yet. Re-run install.bat to set it up.
)
echo.
echo Next:  export.bat   then   cd Getting_Started\01-Hello_Blink\Sw   then   tdf flash monitor
exit /b 0

rem ====================== subroutines ======================

:check_python
call :find_python
if not defined PYTHON (
    echo   [ ] python3 -- not found ^(runs tdf.py itself^)
    set "MISSING=!MISSING! python"
    exit /b 0
)
set "_pv="
for /f "delims=" %%v in ('!PYTHON! --version 2^>^&1') do if not defined _pv set "_pv=%%v"
echo   [x] python3 -- !_pv!
exit /b 0

:check_tool
where %1 >nul 2>&1
if errorlevel 1 (
    echo   [ ] %1 -- not found
    set "MISSING=!MISSING! %1"
) else (
    set "_v="
    for /f "delims=" %%v in ('%1 --version 2^>^&1') do if not defined _v set "_v=%%v"
    echo   [x] %1 -- !_v!
)
exit /b 0

:find_python
rem Resolve a *working* Python 3, not just any name on PATH: the bare
rem "python" Microsoft Store alias prints nothing and exits non-zero for
rem --version, so a stdout version line is the real test.
set "PYTHON="
for /f "delims=" %%v in ('py -3 --version 2^>nul') do set "PYTHON=py -3"
if not defined PYTHON for /f "delims=" %%v in ('python --version 2^>nul') do set "PYTHON=python"
exit /b 0

:install_tool
if /i "%1"=="python" (
    if "%MGR%"=="winget" (
        echo python3: winget catalog ids are minor-version pinned; using 3.13.
        call :confirm "winget install Python.Python.3.13" && winget install --id Python.Python.3.13 -e --accept-package-agreements --accept-source-agreements
    ) else (
        call :confirm "choco install python" && choco install python -y
    )
)
if /i "%1"=="cmake" (
    if "%MGR%"=="winget" (
        call :confirm "winget install Kitware.CMake" && winget install --id Kitware.CMake -e --accept-package-agreements --accept-source-agreements
    ) else (
        call :confirm "choco install cmake" && choco install cmake -y
    )
)
if /i "%1"=="ninja" (
    if "%MGR%"=="winget" (
        call :confirm "winget install Ninja-build.Ninja" && winget install --id Ninja-build.Ninja -e --accept-package-agreements --accept-source-agreements
    ) else (
        call :confirm "choco install ninja" && choco install ninja -y
    )
)
if /i "%1"=="arm-none-eabi-gcc" (
    echo.
    echo arm-none-eabi-gcc: the Arm GNU bare-metal toolchain package id has changed
    echo names over the years, so this script will not guess. Options:
    if "%MGR%"=="winget" (
        echo   1^) winget install Arm.GnuArmEmbeddedToolchain   ^(older id, may still work^)
    ) else (
        echo   1^) choco install gcc-arm-embedded
    )
    echo   2^) Official installer: search "Arm GNU Toolchain Downloads" on developer.arm.com,
    echo      pick the AArch32 bare-metal ^(arm-none-eabi^) Windows build, and tick
    echo      "Add path to environment variable" in the installer.
    if "%MGR%"=="winget" (
        call :confirm "winget install Arm.GnuArmEmbeddedToolchain (option 1)" && winget install --id Arm.GnuArmEmbeddedToolchain -e --accept-package-agreements --accept-source-agreements
    ) else (
        call :confirm "choco install gcc-arm-embedded (option 1)" && choco install gcc-arm-embedded -y
    )
)
exit /b 0

:confirm
if "%ASSUME_YES%"=="1" exit /b 0
set "reply="
set /p "reply=Run: %~1 ? [y/N] "
if /i "%reply%"=="y" exit /b 0
if /i "%reply%"=="yes" exit /b 0
echo   skipped
exit /b 1
