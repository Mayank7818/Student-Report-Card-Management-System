@echo off
setlocal enabledelayedexpansion
title Student Report Card Management System

rem ---------------------------------------------------------------------------
rem  Builds and launches the Student Report Card Management System.
rem
rem  Usage:  start.bat ["C:\Path\To\MySQL Server 8.0"]
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

rem --- 1. compiler -----------------------------------------------------------
where g++ >nul 2>&1
if errorlevel 1 (
    echo [ERROR] g++ was not found on your PATH.
    echo         Install MinGW-w64 ^(64-bit^) and add its \bin folder to PATH.
    echo.
    pause
    exit /b 1
)

for /f "delims=" %%A in ('g++ -dumpmachine 2^>nul') do set "TRIPLE=%%A"
echo [1/4] Compiler target : !TRIPLE!

echo !TRIPLE! | findstr /i "x86_64" >nul
if errorlevel 1 (
    echo.
    echo [WARN] This g++ builds 32-bit binaries, but MySQL 8.0 ships 64-bit
    echo        libraries. Linking will fail with "file format not recognized".
    echo        Fix: install MinGW-w64 ^(x86_64^) and put it first on PATH.
    echo.
)

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
if exist "schema.sql" (
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
g++ -std=c++11 -Wall -O2 -I"%MYSQL_DIR%\include" %SOURCES% -o "%APP%" -L"%MYSQL_DIR%\lib" -lmysql
if errorlevel 1 (
    echo.
    echo [ERROR] Build failed. See the messages above.
    echo.
    pause
    exit /b 1
)

if not exist "libmysql.dll" (
    if exist "%MYSQL_DIR%\lib\libmysql.dll" copy /y "%MYSQL_DIR%\lib\libmysql.dll" . >nul
)

rem --- 5. run ----------------------------------------------------------------
echo [4/4] Starting ...
echo.
"%APP%"

echo.
echo Application exited with code %errorlevel%.
pause
endlocal
