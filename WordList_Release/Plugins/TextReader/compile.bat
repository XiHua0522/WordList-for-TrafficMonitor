@echo off
REM Compile script for TextReader plugin using CMake
REM Usage: run in Administrator developer command prompt or where VS/CMake are available

setlocal
echo ====================================
echo TextReader CMake build script
echo ====================================

where cmake >nul 2>nul
if errorlevel 1 (
    echo [ERROR] cmake not found. Please install CMake and ensure it's on PATH.
    exit /b 1
)

set BUILD_DIR=build_cmake
if not exist %BUILD_DIR% mkdir %BUILD_DIR%
pushd %BUILD_DIR%

echo [Step] Configuring with CMake (try Visual Studio generator)...
cmake .. -G "Visual Studio 17 2022" -A Win32 > cmake_config.log 2>&1
if errorlevel 1 (
    echo [WARN] VS2022 generator failed, trying default generator...
    type cmake_config.log
    cmake .. > cmake_config2.log 2>&1
    if errorlevel 1 (
        echo [ERROR] CMake configure failed. See cmake_config2.log
        popd
        exit /b 1
    )
)

echo [Step] Building (Release)...
cmake --build . --config Release -- /m > cmake_build.log 2>&1
if errorlevel 1 (
    echo [ERROR] Build failed. See cmake_build.log for details.
    popd
    exit /b 1
)

echo [SUCCESS] Build finished.
echo Output directory: %CD%\bin (or check project OutDir setting)
popd
endlocal
pause
