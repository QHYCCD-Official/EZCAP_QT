@echo off
setlocal EnableExtensions EnableDelayedExpansion

REM Usage:
REM   deploy_qt_runtime.bat [debug|release] [build] [run]
REM Example:
REM   deploy_qt_runtime.bat debug
REM   deploy_qt_runtime.bat debug run
REM   deploy_qt_runtime.bat debug build run

set "SCRIPT_DIR=%~dp0"
if "%SCRIPT_DIR:~-1%"=="\" set "SCRIPT_DIR=%SCRIPT_DIR:~0,-1%"

set "PROJECT_ROOT=%SCRIPT_DIR%"
if not exist "%PROJECT_ROOT%\EZCAP.pro" (
    if exist "%PROJECT_ROOT%\..\EZCAP.pro" (
        for %%I in ("%PROJECT_ROOT%\..") do set "PROJECT_ROOT=%%~fI"
    )
)

if not exist "%PROJECT_ROOT%\EZCAP.pro" (
    echo [ERROR] Project root not found from script location.
    echo [HINT] Put script under project root or project_root\script.
    exit /b 9
)

set "CONFIG=debug"
set "BUILD_FIRST=0"
set "RUN_EXE=0"
for %%A in (%*) do (
    if /I "%%~A"=="debug" set "CONFIG=debug"
    if /I "%%~A"=="release" set "CONFIG=release"
    if /I "%%~A"=="build" set "BUILD_FIRST=1"
    if /I "%%~A"=="run" set "RUN_EXE=1"
)

set "TARGET_DIR=%PROJECT_ROOT%\build\%CONFIG%"
set "TARGET_EXE=%TARGET_DIR%\EZCAP.exe"

if "%BUILD_FIRST%"=="0" if not exist "%TARGET_EXE%" (
    echo [INFO] Target not found: "%TARGET_EXE%"
    echo [INFO] Auto enable build step.
    set "BUILD_FIRST=1"
)

set "QMAKE_EXE="
set "MAKEFILE_PATH=%PROJECT_ROOT%\build\Makefile"
if exist "%MAKEFILE_PATH%" (
    for /f "tokens=1,* delims==" %%A in ('findstr /B /C:"QMAKE         =" "%MAKEFILE_PATH%"') do (
        set "QMAKE_EXE=%%B"
    )
)

if defined QMAKE_EXE (
    for /f "tokens=* delims= " %%I in ("!QMAKE_EXE!") do set "QMAKE_EXE=%%I"
)

set "QT_BIN="
if defined QMAKE_EXE (
    for %%I in ("!QMAKE_EXE!") do set "QT_BIN=%%~dpI"
)

if not defined QMAKE_EXE if defined QTDIR (
    if exist "%QTDIR%\bin\qmake.exe" set "QMAKE_EXE=%QTDIR%\bin\qmake.exe"
)

if not defined QMAKE_EXE (
    if exist "C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin\qmake.exe" set "QMAKE_EXE=C:\Qt\Qt5.14.2\5.14.2\mingw73_64\bin\qmake.exe"
)

if not defined QMAKE_EXE (
    if exist "C:\Qt\Qt5.14.2\5.14.2\mingw73_32\bin\qmake.exe" set "QMAKE_EXE=C:\Qt\Qt5.14.2\5.14.2\mingw73_32\bin\qmake.exe"
)

if not defined QMAKE_EXE (
    for /f "delims=" %%I in ('where qmake.exe 2^>nul') do (
        set "QMAKE_EXE=%%I"
        goto :qmake_found
    )
)

if not defined QMAKE_EXE if exist "C:\Qt" (
    for /r "C:\Qt" %%I in (qmake.exe) do (
        if exist "%%~fI" (
            set "QMAKE_EXE=%%~fI"
            goto :qmake_found
        )
    )
)

if not defined QMAKE_EXE if exist "D:\Qt" (
    for /r "D:\Qt" %%I in (qmake.exe) do (
        if exist "%%~fI" (
            set "QMAKE_EXE=%%~fI"
            goto :qmake_found
        )
    )
)

if not defined QMAKE_EXE if exist "E:\Qt" (
    for /r "E:\Qt" %%I in (qmake.exe) do (
        if exist "%%~fI" (
            set "QMAKE_EXE=%%~fI"
            goto :qmake_found
        )
    )
)

:qmake_found
if not defined QT_BIN if defined QMAKE_EXE (
    for %%I in ("!QMAKE_EXE!") do set "QT_BIN=%%~dpI"
)

if not defined QT_BIN if defined QTDIR (
    if exist "%QTDIR%\bin\windeployqt.exe" set "QT_BIN=%QTDIR%\bin\"
)

if not defined QT_BIN if defined QMAKE_EXE (
    for %%I in ("!QMAKE_EXE!") do (
        if exist "%%~dpIwindeployqt.exe" set "QT_BIN=%%~dpI"
    )
)

if not defined QT_BIN if exist "C:\Qt" (
    for /r "C:\Qt" %%I in (windeployqt.exe) do (
        if exist "%%~fI" (
            set "QT_BIN=%%~dpI"
            goto :qt_bin_found
        )
    )
)

if not defined QT_BIN if exist "D:\Qt" (
    for /r "D:\Qt" %%I in (windeployqt.exe) do (
        if exist "%%~fI" (
            set "QT_BIN=%%~dpI"
            goto :qt_bin_found
        )
    )
)

if not defined QT_BIN if exist "E:\Qt" (
    for /r "E:\Qt" %%I in (windeployqt.exe) do (
        if exist "%%~fI" (
            set "QT_BIN=%%~dpI"
            goto :qt_bin_found
        )
    )
)

:qt_bin_found

set "ARCH=64"
echo %QT_BIN% | findstr /I "_32" >nul && set "ARCH=32"

set "DEPEND_DIR=%PROJECT_ROOT%\Depend\x64"
if "%ARCH%"=="32" set "DEPEND_DIR=%PROJECT_ROOT%\Depend\x86"

set "QT_VERSION_ROOT="
for %%I in ("%QT_BIN%..\..") do set "QT_VERSION_ROOT=%%~fI"

set "TOOLS_ROOT="
for %%I in ("%QT_VERSION_ROOT%\..\Tools") do set "TOOLS_ROOT=%%~fI"

set "MINGW_BIN="
for /d %%D in (%TOOLS_ROOT%\mingw*_%ARCH%) do (
    if exist "%%~fD\bin\g++.exe" (
        set "MINGW_BIN=%%~fD\bin\"
        goto :mingw_found
    )
)
:mingw_found

if not defined MINGW_BIN (
    if exist "%TOOLS_ROOT%\mingw730_%ARCH%\bin\g++.exe" set "MINGW_BIN=%TOOLS_ROOT%\mingw730_%ARCH%\bin\"
)

if not defined MINGW_BIN (
    if exist "C:\Qt\Qt5.14.2\Tools\mingw730_%ARCH%\bin\g++.exe" set "MINGW_BIN=C:\Qt\Qt5.14.2\Tools\mingw730_%ARCH%\bin\"
)

if not defined MINGW_BIN if exist "C:\Qt" (
    for /r "C:\Qt" %%I in (mingw32-make.exe) do (
        if exist "%%~fI" (
            set "MINGW_BIN=%%~dpI"
            goto :mingw_found_by_search
        )
    )
)

if not defined MINGW_BIN if exist "D:\Qt" (
    for /r "D:\Qt" %%I in (mingw32-make.exe) do (
        if exist "%%~fI" (
            set "MINGW_BIN=%%~dpI"
            goto :mingw_found_by_search
        )
    )
)

if not defined MINGW_BIN if exist "E:\Qt" (
    for /r "E:\Qt" %%I in (mingw32-make.exe) do (
        if exist "%%~fI" (
            set "MINGW_BIN=%%~dpI"
            goto :mingw_found_by_search
        )
    )
)

:mingw_found_by_search

set "MAKE_EXE=mingw32-make.exe"
if defined MINGW_BIN if exist "%MINGW_BIN%mingw32-make.exe" set "MAKE_EXE=%MINGW_BIN%mingw32-make.exe"
set "MAKE_JOBS=1"
if defined EZCAP_MAKE_JOBS set "MAKE_JOBS=%EZCAP_MAKE_JOBS%"

if "%BUILD_FIRST%"=="1" (
    echo ========================================
    echo Building project first: %CONFIG%
    echo ========================================
    if not defined QMAKE_EXE (
        echo [ERROR] qmake path not found, cannot build.
        echo [HINT] Build once in Qt Creator or set QTDIR.
        exit /b 6
    )
    if not defined MINGW_BIN (
        echo [WARN] MinGW g++ not found automatically, build may fail.
    )
    if defined MINGW_BIN (
        set "PATH=%MINGW_BIN%;%QT_BIN%;%PATH%"
        set "CC=%MINGW_BIN%gcc.exe"
        set "CXX=%MINGW_BIN%g++.exe"
    ) else (
        set "PATH=%QT_BIN%;%PATH%"
    )
    if not exist "%PROJECT_ROOT%\build" mkdir "%PROJECT_ROOT%\build"
    pushd "%PROJECT_ROOT%\build"
    if defined MINGW_BIN (
        "%QMAKE_EXE%" "%PROJECT_ROOT%\EZCAP.pro" -spec win32-g++ "CONFIG+=%CONFIG%" "QMAKE_CC=%MINGW_BIN%gcc.exe" "QMAKE_CXX=%MINGW_BIN%g++.exe" "QMAKE_LINK=%MINGW_BIN%g++.exe" "QMAKE_LINK_C=%MINGW_BIN%gcc.exe"
    ) else (
        "%QMAKE_EXE%" "%PROJECT_ROOT%\EZCAP.pro" -spec win32-g++ "CONFIG+=%CONFIG%"
    )
    if errorlevel 1 (
        popd
        echo [ERROR] qmake failed.
        exit /b 7
    )
    "%MAKE_EXE%" -j%MAKE_JOBS%
    if errorlevel 1 (
        popd
        echo [ERROR] build failed.
        exit /b 8
    )
    popd
)

if not exist "%TARGET_EXE%" (
    echo [ERROR] Target not found: "%TARGET_EXE%"
    echo [HINT] Build first or check build output directory.
    exit /b 2
)

if not defined QT_BIN (
    echo [ERROR] Qt bin path not found.
    echo [HINT] Re-run qmake in Qt Creator, or set QTDIR first.
    exit /b 3
)

set "WINDEPLOYQT=%QT_BIN%windeployqt.exe"
if not exist "%WINDEPLOYQT%" (
    echo [ERROR] windeployqt not found: "%WINDEPLOYQT%"
    exit /b 4
)

echo ========================================
echo Deploying Qt runtime for %CONFIG% (%ARCH%-bit)
echo Target: %TARGET_EXE%
echo Qt bin: %QT_BIN%
echo ========================================

"%WINDEPLOYQT%" --force "%TARGET_EXE%"
if errorlevel 1 (
    echo [ERROR] windeployqt failed.
    exit /b 5
)

if exist "%QT_BIN%libgcc_s_seh-1.dll" copy /Y "%QT_BIN%libgcc_s_seh-1.dll" "%TARGET_DIR%\" >nul
if exist "%QT_BIN%libstdc++-6.dll" copy /Y "%QT_BIN%libstdc++-6.dll" "%TARGET_DIR%\" >nul
if exist "%QT_BIN%libwinpthread-1.dll" copy /Y "%QT_BIN%libwinpthread-1.dll" "%TARGET_DIR%\" >nul

if defined MINGW_BIN (
    if exist "%MINGW_BIN%libgcc_s_seh-1.dll" copy /Y "%MINGW_BIN%libgcc_s_seh-1.dll" "%TARGET_DIR%\" >nul
    if exist "%MINGW_BIN%libstdc++-6.dll" copy /Y "%MINGW_BIN%libstdc++-6.dll" "%TARGET_DIR%\" >nul
    if exist "%MINGW_BIN%libwinpthread-1.dll" copy /Y "%MINGW_BIN%libwinpthread-1.dll" "%TARGET_DIR%\" >nul
)

if exist "%DEPEND_DIR%\*.dll" copy /Y "%DEPEND_DIR%\*.dll" "%TARGET_DIR%\" >nul

set "QHYCCD_DLL_SRC="
if exist "%SCRIPT_DIR%\qhyccd.dll" set "QHYCCD_DLL_SRC=%SCRIPT_DIR%\qhyccd.dll"
if not defined QHYCCD_DLL_SRC if exist "%PROJECT_ROOT%\script\qhyccd.dll" set "QHYCCD_DLL_SRC=%PROJECT_ROOT%\script\qhyccd.dll"
if not defined QHYCCD_DLL_SRC if exist "%PROJECT_ROOT%\qhyccd.dll" set "QHYCCD_DLL_SRC=%PROJECT_ROOT%\qhyccd.dll"

if defined QHYCCD_DLL_SRC (
    copy /Y "%QHYCCD_DLL_SRC%" "%TARGET_DIR%\" >nul
    echo [INFO] qhyccd.dll copied from "%QHYCCD_DLL_SRC%"
) else (
    echo [WARN] qhyccd.dll not found in script/project root.
)

set "WINDOWS_PLUGIN=qwindows.dll"
if /I "%CONFIG%"=="debug" set "WINDOWS_PLUGIN=qwindowsd.dll"
if not exist "%TARGET_DIR%\platforms\%WINDOWS_PLUGIN%" (
    echo [WARN] platforms\qwindows.dll missing after deploy.
)

echo [OK] Deploy finished: "%TARGET_DIR%"
if "%RUN_EXE%"=="1" (
    echo [INFO] Starting "%TARGET_EXE%"
    start "" "%TARGET_EXE%"
)
exit /b 0
