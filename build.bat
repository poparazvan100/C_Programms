@echo off
REM Builds the web account records console app.
REM Requires MinGW g++ (GCC 6.3.0) on PATH, or installed at C:\MinGW\bin.
REM
REM C++14, not C++17: GCC 6.3.0 ships a pre-C++17 libstdc++, so <optional>,
REM <string_view> and <filesystem> are missing and fail at #include.

setlocal

set GXX=g++
where %GXX% >nul 2>&1
if errorlevel 1 (
    if exist "C:\MinGW\bin\g++.exe" (
        set GXX=C:\MinGW\bin\g++.exe
    ) else (
        echo ERROR: g++ not found on PATH or at C:\MinGW\bin\g++.exe
        exit /b 1
    )
)

echo Compiling main.cpp...
"%GXX%" -std=c++14 -Wall -Wextra main.cpp -o app.exe
if errorlevel 1 (
    echo BUILD FAILED
    exit /b 1
)

echo Built app.exe
endlocal
