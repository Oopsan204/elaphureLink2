@echo off
echo ===================================
echo Xây dựng và test auto-reconnect
echo ===================================

echo.
echo Bước 1: Build elaphureLinkProxy project...
"%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" elaphureLinkProxy\elaphureLinkProxy.vcxproj /p:Configuration=Debug /p:Platform=x64
if errorlevel 1 (
    echo ERROR: Không thể build elaphureLinkProxy project
    pause
    exit /b 1
)

echo.
echo Bước 2: Build test file...
cl /EHsc /I. test_auto_reconnect.cpp /link bin\x64\Debug\elaphureLinkProxy.lib /out:test_auto_reconnect.exe
if errorlevel 1 (
    echo ERROR: Không thể build test file
    pause
    exit /b 1
)

echo.
echo Bước 3: Chạy test auto-reconnect...
test_auto_reconnect.exe

echo.
echo Test hoàn thành!
pause
