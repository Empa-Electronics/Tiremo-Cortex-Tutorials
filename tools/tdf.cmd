@echo off
rem Windows shim so `tdf ...` works from cmd.exe and PowerShell once tools\
rem is on PATH (see export.bat / export.ps1), the same way the `tdf.py`
rem shebang lets it run bare on macOS/Linux. `python tools\tdf.py ...`
rem works identically without this.
rem
rem Prefers tdf_venv\ (pyserial, for `tdf.py monitor`) when it exists, so
rem `tdf monitor` works even if you never ran export.bat / export.ps1.
setlocal
set "TDF_DIR=%~dp0"
set "TDF_VENV_PY=%TDF_DIR%..\tdf_venv\Scripts\python.exe"

if exist "%TDF_VENV_PY%" (
    "%TDF_VENV_PY%" "%TDF_DIR%tdf.py" %*
    exit /b %ERRORLEVEL%
)

where py >nul 2>&1
if %ERRORLEVEL%==0 (
    py -3 "%TDF_DIR%tdf.py" %*
    exit /b %ERRORLEVEL%
)

python "%TDF_DIR%tdf.py" %*
exit /b %ERRORLEVEL%
