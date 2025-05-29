@echo off
REM Auto-Reconnect Feature Build and Test Script
REM Make sure to have Visual Studio environment set up

echo === Building elaphureLink with Auto-Reconnect ===

REM Change to solution directory
cd /d "e:\code\robocon2025\elaphureLink2"

REM Build the solution
echo Building solution...
msbuild elaphureLink.sln /p:Configuration=Release /p:Platform=x64 /v:minimal

if %ERRORLEVEL% neq 0 (
    echo Build failed!
    pause
    exit /b 1
)

echo Build successful!

REM Optional: Run auto-reconnect test if test executable exists
if exist "test\proxy_test\auto_reconnect_test.exe" (
    echo.
    echo Running auto-reconnect test...
    cd test\proxy_test
    auto_reconnect_test.exe
    cd ..\..
) else (
    echo.
    echo Auto-reconnect test not found. Build test project separately if needed.
)

echo.
echo === Build Complete ===
echo Auto-reconnect feature has been integrated into elaphureLink.
echo.
echo Key new APIs:
echo - el_proxy_enable_auto_reconnect(config)
echo - el_proxy_disable_auto_reconnect()
echo - el_proxy_is_auto_reconnect_enabled()
echo - el_proxy_get_reconnect_attempt_count()
echo - el_proxy_set_on_reconnect_attempt_callback(callback)
echo - el_proxy_set_on_reconnect_failed_callback(callback)
echo.
echo See docs\auto_reconnect.md for detailed documentation.

pause
