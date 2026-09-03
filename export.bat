@echo off
rem Windows (cmd.exe) equivalent of export.sh. Just RUN it -- there is no
rem `setlocal` here on purpose, so it edits THIS console's environment:
rem
rem     export.bat
rem
rem Adds tools\ to PATH so `tdf` runs bare from any directory, and activates
rem tdf_venv\ so `tdf.py monitor` finds pyserial. `python tools\tdf.py
rem build` works identically without ever running this. PowerShell users:
rem use  . .\export.ps1  instead (it also installs tab completion).

set "TDF_PATH=%~dp0"
if "%TDF_PATH:~-1%"=="\" set "TDF_PATH=%TDF_PATH:~0,-1%"
set "TDF_TOOLS=%TDF_PATH%\tools"

echo %PATH% | find /i "%TDF_TOOLS%" >nul
if errorlevel 1 (
    set "PATH=%TDF_TOOLS%;%PATH%"
    echo Tiremo Development Framework: tools\ added to PATH ^(tdf / tdf.py^).
) else (
    echo Tiremo Development Framework: tools\ already on PATH.
)

if exist "%TDF_PATH%\tdf_venv\Scripts\activate.bat" (
    set "VIRTUAL_ENV_DISABLE_PROMPT=1"
    call "%TDF_PATH%\tdf_venv\Scripts\activate.bat"
    echo tdf_venv activated ^(pyserial available for 'tdf.py monitor'^).
) else (
    echo No tdf_venv\ found -- 'tdf.py monitor' needs pyserial. Run install.bat once to set it up.
)

echo.
echo Note: cmd.exe has no command-name tab completion, so `tdf ^<Tab^>` will
echo not list subcommands. Use PowerShell with  . .\export.ps1  if you want that.
echo.
echo Try: cd Getting_Started\01-Hello_Blink\Sw ^&^& tdf flash monitor
