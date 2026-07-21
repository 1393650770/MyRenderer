@echo off
setlocal enabledelayedexpansion

REM Android SDK paths — edit these two for your machine
set "ANDROID_SDK_ROOT=D:\Project\AndroidSDK"
set "JDK_ROOT=D:\Project\AndroidSDK\jdk\jdk21"

REM derived paths
set "BUILD_TOOLS=%ANDROID_SDK_ROOT%\build-tools\34.0.0"
set "ANDROID_JAR=%ANDROID_SDK_ROOT%\platforms\android-35\android.jar"
set "PATH=%BUILD_TOOLS%;%JDK_ROOT%\bin;%PATH%"

REM project paths (auto-detected)
set "APK_DIR=%~dp0"
set "PROJECT_DIR=%APK_DIR%..\.."
set "BUILD_DIR=%PROJECT_DIR%\build\android\arm64-v8a\release"
set "OUT_APK=%APK_DIR%mxrender_hello.apk"
set "MANIFEST=%APK_DIR%AndroidManifest.xml"

echo === MXRender APK Builder ===

set "TMPDIR=%APK_DIR%tmp_apk"
if exist "%TMPDIR%" rmdir /s /q "%TMPDIR%"
mkdir "%TMPDIR%\lib\arm64-v8a"

echo Copying .so files...
copy "%BUILD_DIR%\libRendererSample-HelloTriangle.so" "%TMPDIR%\lib\arm64-v8a\" >nul
copy "%BUILD_DIR%\libRuntime.so" "%TMPDIR%\lib\arm64-v8a\" >nul

echo Copying imgui...
set "FOUND_IMGUI=0"
set "IMGUI_BASE=%LOCALAPPDATA%\.xmake\cache\packages"
if exist "%IMGUI_BASE%" (
    for /d %%a in ("%IMGUI_BASE%\*") do (
        for /d %%b in ("%%a\i\imgui\v1.89.9-docking\source\imgui\build_*") do (
            for /d %%c in ("%%b\android\arm64-v8a\*") do (
                if exist "%%c\libimgui.so" (
                    copy "%%c\libimgui.so" "%TMPDIR%\lib\arm64-v8a\" >nul
                    set "FOUND_IMGUI=1"
                )
            )
        )
    )
)
if "!FOUND_IMGUI!"=="0" (
    echo ERROR: libimgui.so not found! Run xmake build first.
    goto :error
)

echo Copying shader assets...
mkdir "%TMPDIR%\assets\Shader" 2>nul
for /r "%PROJECT_DIR%\resource\Shader" %%s in (*.spv) do (
    copy "%%s" "%TMPDIR%\assets\Shader\" >nul 2>&1
)

echo Linking APK...
"%BUILD_TOOLS%\aapt2.exe" link -o "%TMPDIR%\base.apk" --manifest "%MANIFEST%" -I "%ANDROID_JAR%"
if errorlevel 1 goto :error

echo Adding .so to APK...
cd /d "%TMPDIR%"
for %%s in (lib\arm64-v8a\*.so) do (
    "%JDK_ROOT%\bin\jar" uf base.apk "%%s"
)
if exist assets\Shader\*.spv (
    "%JDK_ROOT%\bin\jar" uf base.apk assets/
)
if exist assets\Texture (
    "%JDK_ROOT%\bin\jar" uf base.apk assets/
)

echo Aligning...
"%BUILD_TOOLS%\zipalign.exe" -f 4 base.apk aligned.apk

echo Signing...
"%BUILD_TOOLS%\apksigner.bat" sign --ks "%APK_DIR%debug.keystore" --ks-pass pass:android --min-sdk-version 26 --out "%OUT_APK%" aligned.apk
if errorlevel 1 goto :error

echo.
echo === APK created: %OUT_APK% ===
echo Install: adb install -r "%OUT_APK%"
goto :end

:error
echo ERROR: APK build failed!
exit /b 1
:end
endlocal
