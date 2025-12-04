@echo off
REM Generate Visual Studio 2022 solution with dependencies
REM Run this script from the project root directory

echo ================================================
echo  cpp_3d_jump - Visual Studio Project Generator
echo ================================================
echo.

REM Check for git (required for vcpkg installation)
where git >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Git not found in PATH.
    echo Please install Git from https://git-scm.com/download/win
    pause
    exit /b 1
)

REM Check for CMake
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found in PATH.
    echo Please install CMake from https://cmake.org/download/
    echo Make sure to select "Add CMake to PATH" during installation.
    pause
    exit /b 1
)

REM Set default vcpkg location
set VCPKG_DEFAULT_DIR=C:\vcpkg

REM Check for vcpkg - try VCPKG_ROOT first, then default location
if defined VCPKG_ROOT (
    if exist "%VCPKG_ROOT%\vcpkg.exe" (
        echo Found vcpkg at: %VCPKG_ROOT%
        goto :vcpkg_ready
    )
)

if exist "%VCPKG_DEFAULT_DIR%\vcpkg.exe" (
    set VCPKG_ROOT=%VCPKG_DEFAULT_DIR%
    echo Found vcpkg at: %VCPKG_ROOT%
    goto :vcpkg_ready
)

REM vcpkg not found - install it
echo vcpkg not found. Installing to %VCPKG_DEFAULT_DIR%...
echo.

echo [1/3] Cloning vcpkg repository...
git clone https://github.com/Microsoft/vcpkg.git "%VCPKG_DEFAULT_DIR%"
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to clone vcpkg repository.
    pause
    exit /b 1
)

echo.
echo [2/3] Bootstrapping vcpkg...
cd /d "%VCPKG_DEFAULT_DIR%"
call bootstrap-vcpkg.bat -disableMetrics
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Failed to bootstrap vcpkg.
    pause
    exit /b 1
)

echo.
echo [3/3] Setting VCPKG_ROOT environment variable...
setx VCPKG_ROOT "%VCPKG_DEFAULT_DIR%"
set VCPKG_ROOT=%VCPKG_DEFAULT_DIR%

echo.
echo ================================================
echo  vcpkg installed successfully!
echo  VCPKG_ROOT set to: %VCPKG_ROOT%
echo ================================================
echo.

:vcpkg_ready
REM Return to project directory
cd /d "%~dp0"

echo.
echo [1/4] Installing dependencies via vcpkg...
echo.

REM Install required packages
echo Installing glfw3...
"%VCPKG_ROOT%\vcpkg.exe" install glfw3:x64-windows
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: glfw3 installation had issues, continuing anyway...
)

echo.
echo Installing freetype...
"%VCPKG_ROOT%\vcpkg.exe" install freetype:x64-windows
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: freetype installation had issues, continuing anyway...
)

echo.
echo [2/4] Integrating vcpkg with Visual Studio...
"%VCPKG_ROOT%\vcpkg.exe" integrate install

echo.
echo [3/4] Creating build directory...
if not exist "build_vs" mkdir build_vs
cd build_vs

echo.
echo [4/4] Generating Visual Studio 2022 solution...
REM Change "Visual Studio 17 2022" to your version:
REM   - "Visual Studio 16 2019" for VS 2019
REM   - "Visual Studio 15 2017" for VS 2017
cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ..

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ================================================
    echo  SUCCESS! Solution generated.
    echo ================================================
    echo.
    echo Open: build_vs\cpp_3d_jump.sln
    echo.
    echo Tips:
    echo   - Set configuration to Debug or Release
    echo   - Press F5 to build and debug
    echo   - Assets are copied to build directory automatically
    echo.
) else (
    echo.
    echo ================================================
    echo  ERROR: CMake generation failed.
    echo ================================================
    echo.
    echo Make sure you have:
    echo   - Visual Studio 2022 with "Desktop development with C++" workload
    echo.
)

cd ..
pause
