@echo off
setlocal enabledelayedexpansion
title Student Report Card Management System

rem ---------------------------------------------------------------------------
rem  Builds and launches the Student Report Card Management System.
rem
rem  Usage:  start.bat ["C:\Path\To\MySQL Server 8.0"]
rem
rem  Set BUILD_ONLY=1 before calling to compile without running.
rem ---------------------------------------------------------------------------

set "APP=report_card.exe"
set "SOURCES=main.cpp Console.cpp Database.cpp Student.cpp"
set "MYSQL_DIR=C:\Program Files\MySQL\MySQL Server 8.0"
if not "%~1"=="" set "MYSQL_DIR=%~1"

cd /d "%~dp0"

echo ====================================================
echo   Student Report Card Management System - Launcher
echo ====================================================
echo.

rem --- 1. find a 64-bit compiler --------------------------------------------
rem MySQL 8.0 ships 64-bit libraries, so a 32-bit g++ cannot link against them.
rem Prefer whatever is on PATH, but fall back to known 64-bit install locations
rem rather than failing when an older 32-bit MinGW shadows it.
set "CXX="

where g++ >nul 2>&1
if not errorlevel 1 (
    for /f "delims=" %%A in ('g++ -dumpmachine 2^>nul') do set "TRIPLE=%%A"
    echo !TRIPLE! | findstr /i "x86_64" >nul
    if not errorlevel 1 set "CXX=g++"
)

if not defined CXX (
    for %%D in (
        "%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.UCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
        "%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs.POSIX.MSVCRT_Microsoft.Winget.Source_8wekyb3d8bbwe\mingw64\bin"
        "C:\msys64\mingw64\bin"
        "C:\mingw64\bin"
        "C:\Program Files\mingw64\bin"
    ) do (
        if not defined CXX if exist "%%~D\g++.exe" set "CXX=%%~D\g++.exe"
    )
)

if not defined CXX (
    echo [ERROR] No 64-bit g++ was found.
    echo.
    echo         MySQL 8.0 ships 64-bit libraries, so a 32-bit compiler cannot
    echo         link against them. Install MinGW-w64 with:
    echo.
    echo             winget install BrechtSanders.WinLibs.POSIX.UCRT
    echo.
    pause
    exit /b 1
)

for /f "delims=" %%A in ('"!CXX!" -dumpmachine 2^>nul') do set "TRIPLE=%%A"
echo [1/4] Compiler       : !TRIPLE!

rem --- 2. MySQL headers ------------------------------------------------------
if not exist "%MYSQL_DIR%\include\mysql.h" (
    echo [ERROR] mysql.h not found under "%MYSQL_DIR%".
    echo         Pass the correct location, for example:
    echo             start.bat "D:\MySQL\MySQL Server 8.0"
    echo.
    pause
    exit /b 1
)
echo [2/4] MySQL headers  : "%MYSQL_DIR%\include"

rem --- 3. optional schema load ----------------------------------------------
if not defined BUILD_ONLY if exist "schema.sql" (
    set "LOADSCHEMA="
    set /p "LOADSCHEMA=      Create/verify the database schema first? (y/N): "
    if /i "!LOADSCHEMA!"=="y" (
        if exist "%MYSQL_DIR%\bin\mysql.exe" (
            echo       Running schema.sql ...
            "%MYSQL_DIR%\bin\mysql.exe" -u root -p < schema.sql
        ) else (
            echo       [SKIP] mysql.exe not found in "%MYSQL_DIR%\bin".
        )
    )
)

rem --- 4. build --------------------------------------------------------------
echo [3/4] Building %APP% ...
"!CXX!" -std=c++11 -Wall -O2 -I"%MYSQL_DIR%\include" %SOURCES% -o "%APP%" -L"%MYSQL_DIR%\lib" -lmysql
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed. See the messages above.
    echo.
    if not defined BUILD_ONLY pause
    exit /b 1
)
echo       OK - %APP%

if not exist "libmysql.dll" (
    if exist "%MYSQL_DIR%\lib\libmysql.dll" copy /y "%MYSQL_DIR%\lib\libmysql.dll" . >nul
)

if defined BUILD_ONLY (
    echo [4/4] BUILD_ONLY set - not starting.
    exit /b 0
)

rem --- 5. run ----------------------------------------------------------------
echo [4/4] Starting ...
echo.
"%APP%"

echo.
echo Application exited with code %errorlevel%.
pause
endlocal
