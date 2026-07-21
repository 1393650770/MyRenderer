@echo off
setlocal
set "BUILD_TOOLS=D:\Project\AndroidSDK\build-tools\34.0.0"
set "PLATFORM_TOOLS=D:\Project\AndroidSDK\platform-tools\platform-tools"
set "JDK_BIN=D:\Project\AndroidSDK\jdk\jdk21\bin"
set "PATH=%BUILD_TOOLS%;%JDK_BIN%;%PATH%"

set "APK_DIR=%~dp0"
set "PROJECT_DIR=%APK_DIR%..\.."
set "BUILD_DIR=%PROJECT_DIR%\build\android\arm64-v8a\release"
set "OUT_APK=%APK_DIR%mxrender_hello.apk"
set "MANIFEST=%APK_DIR%AndroidManifest.xml"
set "ANDROID_JAR=D:\Project\AndroidSDK\platforms\android-35\android.jar"

echo === MXRender APK Builder ===

set "TMPDIR=%APK_DIR%tmp_apk"
if exist "%TMPDIR%" rmdir /s /q "%TMPDIR%"
mkdir "%TMPDIR%\lib\arm64-v8a"

echo Copying .so files...
REM Copy main .so and Runtime
copy "%BUILD_DIR%\libRendererSample-HelloTriangle.so" "%TMPDIR%\lib\arm64-v8a\" >nul
copy "%BUILD_DIR%\libRuntime.so" "%TMPDIR%\lib\arm64-v8a\" >nul
REM Copy dependency .so (from xmake cache)
for /d %%d in ("%LOCALAPPDATA%\.xmake\cache\packages\*\i\imgui\*\source\imgui\build_*\android\arm64-v8a\*") do (
    if exist "%%d\libimgui.so" copy "%%d\libimgui.so" "%TMPDIR%\lib\arm64-v8a\" >nul
)
REM Also check the installed package dir
for /d %%d in ("%LOCALAPPDATA%\.xmake\packages\i\imgui\*\*\lib") do (
    if exist "%%d\libimgui.so" copy "%%d\libimgui.so" "%TMPDIR%\lib\arm64-v8a\" >nul
)
REM Also copy from the known cache location
if exist "%LOCALAPPDATA%\.xmake\cache\packages\2607\i\imgui\v1.89.9-docking\source\imgui\build_2a632381\android\arm64-v8a\debug\libimgui.so" (
    copy "%LOCALAPPDATA%\.xmake\cache\packages\2607\i\imgui\v1.89.9-docking\source\imgui\build_2a632381\android\arm64-v8a\debug\libimgui.so" "%TMPDIR%\lib\arm64-v8a\" >nul
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
REM Add all .so files
for %%s in (lib\arm64-v8a\*.so) do (
    "%JDK_BIN%\jar" uf base.apk "%%s"
)
REM Add assets (shader, textures)
if exist assets\Shader\*.spv (
    "%JDK_BIN%\jar" uf base.apk assets/
)
if exist assets\Texture (
    "%JDK_BIN%\jar" uf base.apk assets/
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
