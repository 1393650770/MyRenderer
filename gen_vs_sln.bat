@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

echo ========================================
echo   MyRenderer - Generate VS Solution
echo ========================================

REM -- Locate xmake-repo --
set "REPO="
for /d %%r in ("%LOCALAPPDATA%\.xmake\repositories\xmake-repo") do set "REPO=%%r"
if not defined REPO (
    echo [ERROR] xmake-repo not found!
    echo Please install xmake first: https://xmake.io
    pause
    exit /b 1
)
set "GLI_RECIPE=%REPO%\packages\g\gli\xmake.lua"
if not exist "%GLI_RECIPE%" (
    echo [ERROR] gli recipe not found: %GLI_RECIPE%
    pause
    exit /b 1
)

REM -- Apply gli patch --
echo [1/5] Patching gli recipe (fix make_vec4 ambiguity)...
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0patch_gli.ps1" "%GLI_RECIPE%"
if %ERRORLEVEL% NEQ 0 echo   [WARNING] gli patch failed, will try building anyway...

REM -- Apply imgui patch --
echo [2/5] Patching imgui recipe (fix Vulkan loader linking)...
set "IMGUI_RECIPE=%REPO%\packages\i\imgui\xmake.lua"
if exist "%IMGUI_RECIPE%" (
    powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0patch_imgui.ps1" "%IMGUI_RECIPE%"
    if %ERRORLEVEL% NEQ 0 echo   [WARNING] imgui patch failed, will try building anyway...
) else (
    echo   [WARNING] imgui recipe not found, skip patch.
)

REM -- Clear stale caches --
echo [3/5] Clearing stale package caches...
for /d %%i in ("%LOCALAPPDATA%\.xmake\packages\g\gli") do (
    echo   Removing %%i
    rmdir /s /q "%%i" 2>nul
)
for /d %%i in ("%LOCALAPPDATA%\.xmake\cache\packages\*\g\gli") do (
    echo   Removing %%i
    rmdir /s /q "%%i" 2>nul
)
for /d %%i in ("%LOCALAPPDATA%\.xmake\packages\i\imgui") do (
    echo   Removing %%i
    rmdir /s /q "%%i" 2>nul
)
for /d %%i in ("%LOCALAPPDATA%\.xmake\cache\packages\*\i\imgui") do (
    echo   Removing %%i
    rmdir /s /q "%%i" 2>nul
)

REM -- Clean xmake config --
echo [4/5] Cleaning xmake config...
xmake f -c -a >nul 2>&1

REM -- Generate VS solution --
echo [5/5] Generating VS solution...
echo.
xmake project -k vsxmake -m "debug;release;releasedbg" -a x64 -y

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo   Done! VS solution generated.
    echo ========================================
) else (
    echo.
    echo ========================================
    echo   FAILED! See errors above.
    echo.
    echo   Troubleshooting:
    echo   - If boost/assimp download failed:
    echo     set HTTP_PROXY first, or run:
    echo     set HTTP_PROXY=http://127.0.0.1:10809
    echo     Then re-run this script.
    echo   - If other errors, check logs at:
    echo     %%LOCALAPPDATA%%\.xmake\cache\packages\
    echo ========================================
)

pause
