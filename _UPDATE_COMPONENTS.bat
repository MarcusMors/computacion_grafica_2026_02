@echo off
setlocal EnableDelayedExpansion
rem ============================================================
rem  LabCompiler - _UPDATE_COMPONENTS.bat
rem
rem  Downloads and installs the latest versions of the bundled
rem  components. Toggle each component with 1 (update) or
rem  0 (skip) below, then run this file from the project root.
rem
rem  Requires: curl.exe and tar.exe (both included in
rem  Windows 10/11). The glad step additionally needs Python
rem  on PATH (any python.org/MSYS2/MinGW python works).
rem ============================================================

rem ---------------------- TOGGLES -----------------------------
set UPDATE_MINGW=0
set UPDATE_CMAKE=0
set UPDATE_GLM=0
set UPDATE_GLAD=0
set UPDATE_SOIL2=0
rem ------------------------------------------------------------

rem ------------------- VERSIONS / URLS ------------------------
rem Bump these when a newer release comes out (check the
rem download pages), or leave as-is to reinstall the versions
rem below. WinLibs: pick the Win64 UCRT build WITHOUT LLVM/Clang
rem (the compact GCC-only build, ~1.5 GB instead of ~3 GB).
set MINGW_URL=https://github.com/brechtsanders/winlibs_mingw/releases/download/16.2.0posix-14.0.0-ucrt-r1/winlibs-x86_64-posix-seh-gcc-16.2.0-mingw-w64ucrt-14.0.0-r1.zip
set MINGW_DEST_NAME=mingw162_64

set CMAKE_URL=https://github.com/Kitware/CMake/releases/download/v4.4.3/cmake-4.4.3-windows-x86_64.zip

rem GLM is header-only: the inner "glm" folder replaces deps\glm
set GLM_URL=https://github.com/g-truc/glm/archive/refs/tags/1.0.1.zip

set SOIL2_URL=https://github.com/SpartanJ/SOIL2/archive/refs/heads/master.zip
rem ------------------------------------------------------------

set baseDir=%cd%
set tmpDir=%baseDir%\_update_tmp

if not exist "%baseDir%\glfw-master" (
    echo ERROR: run this script from the LabCompiler root folder.
    goto :end
)

echo ============================================================
echo  LabCompiler component updater
echo  MINGW=%UPDATE_MINGW% CMAKE=%UPDATE_CMAKE% GLM=%UPDATE_GLM% GLAD=%UPDATE_GLAD% SOIL2=%UPDATE_SOIL2%
echo ============================================================

mkdir "%tmpDir%" 2>nul

rem ============================================================
if "%UPDATE_MINGW%"=="1" (
    echo.
    echo [MINGW] Downloading %MINGW_URL%
    curl -L -C - -o "%tmpDir%\mingw.zip" "%MINGW_URL%"
    if errorlevel 1 ( echo [MINGW] download FAILED & goto :next1 )
    echo [MINGW] Extracting...
    tar -xf "%tmpDir%\mingw.zip" -C "%tmpDir%"
    if not exist "%tmpDir%\mingw64" ( echo [MINGW] unexpected archive layout & goto :next1 )
    echo [MINGW] Backing up old compiler to MinGW_old_backup
    if exist "%baseDir%\MinGW_old_backup" rmdir /s /q "%baseDir%\MinGW_old_backup"
    move "%baseDir%\MinGW" "%baseDir%\MinGW_old_backup" >nul
    mkdir "%baseDir%\MinGW"
    move "%tmpDir%\mingw64" "%baseDir%\MinGW\%MINGW_DEST_NAME%" >nul
    echo [MINGW] Done. _SET_PATHS_x64.bat auto-detects the new folder.
)
:next1

rem ============================================================
if "%UPDATE_CMAKE%"=="1" (
    echo.
    echo [CMAKE] Downloading %CMAKE_URL%
    curl -L -C - -o "%tmpDir%\cmake.zip" "%CMAKE_URL%"
    if errorlevel 1 ( echo [CMAKE] download FAILED & goto :next2 )
    echo [CMAKE] Extracting...
    tar -xf "%tmpDir%\cmake.zip" -C "%tmpDir%"
    echo [CMAKE] Backing up old CMake to Cmake_old_backup
    if exist "%baseDir%\Cmake_old_backup" rmdir /s /q "%baseDir%\Cmake_old_backup"
    move "%baseDir%\Cmake" "%baseDir%\Cmake_old_backup" >nul
    mkdir "%baseDir%\Cmake"
    for /d %%D in ("%tmpDir%\cmake-*") do move "%%D" "%baseDir%\Cmake\" >nul
    echo [CMAKE] Done. _SET_PATHS_x64.bat auto-detects the new folder.
)
:next2

rem ============================================================
if "%UPDATE_GLM%"=="1" (
    echo.
    echo [GLM] Downloading %GLM_URL%
    curl -L -o "%tmpDir%\glm.zip" "%GLM_URL%"
    if errorlevel 1 ( echo [GLM] download FAILED & goto :next3 )
    tar -xf "%tmpDir%\glm.zip" -C "%tmpDir%"
    echo [GLM] Backing up old GLM to deps\glm_old_backup
    if exist "%baseDir%\glfw-master\deps\glm_old_backup" rmdir /s /q "%baseDir%\glfw-master\deps\glm_old_backup"
    move "%baseDir%\glfw-master\deps\glm" "%baseDir%\glfw-master\deps\glm_old_backup" >nul
    for /d %%D in ("%tmpDir%\glm-*") do move "%%D\glm" "%baseDir%\glfw-master\deps\glm" >nul
    echo [GLM] Done.
)
:next3

rem ============================================================
if "%UPDATE_GLAD%"=="1" (
    echo.
    echo [GLAD] Regenerating glad2 header-only loader ^(gl:compatibility=4.6^)
    where python >nul 2>nul
    if errorlevel 1 (
        echo [GLAD] Python not found on PATH - skipping.
        echo        Install any Python, then run:
        echo          python -m pip install glad2
        echo          python -m glad --api "gl:compatibility=4.6" --extensions "GL_KHR_debug,GL_ARB_multisample,GL_ARB_robustness" --out-path glad2_gen c --header-only
        echo        and copy glad2_gen\include\glad\gl.h over glfw-master\deps\glad\gl.h
    ) else (
        python -m pip install --quiet glad2
        python -m glad --api "gl:compatibility=4.6" --extensions "GL_KHR_debug,GL_ARB_multisample,GL_ARB_robustness" --out-path "%tmpDir%\glad2_gen" c --header-only --reproducible
        if exist "%tmpDir%\glad2_gen\include\glad\gl.h" (
            python "%baseDir%\_patch_glad_header.py" "%tmpDir%\glad2_gen\include\glad\gl.h"
            copy /y "%tmpDir%\glad2_gen\include\glad\gl.h" "%baseDir%\glfw-master\deps\glad\gl.h" >nul
            echo [GLAD] Done.
        ) else (
            echo [GLAD] generation FAILED - keeping existing header.
        )
    )
)

rem ============================================================
if "%UPDATE_SOIL2%"=="1" (
    echo.
    echo [SOIL2] Downloading %SOIL2_URL%
    curl -L -o "%tmpDir%\soil2.zip" "%SOIL2_URL%"
    if errorlevel 1 ( echo [SOIL2] download FAILED & goto :next5 )
    tar -xf "%tmpDir%\soil2.zip" -C "%tmpDir%"
    echo [SOIL2] Replacing deps\SOIL2 source
    rmdir /s /q "%baseDir%\glfw-master\deps\SOIL2"
    mkdir "%baseDir%\glfw-master\deps\SOIL2"
    for /d %%D in ("%tmpDir%\SOIL2-*") do (
        xcopy /e /i /q "%%D\src" "%baseDir%\glfw-master\deps\SOIL2\src" >nul
        xcopy /e /i /q "%%D\cmake" "%baseDir%\glfw-master\deps\SOIL2\cmake" >nul
        copy /y "%%D\CMakeLists.txt" "%baseDir%\glfw-master\deps\SOIL2\" >nul
        copy /y "%%D\LICENSE" "%baseDir%\glfw-master\deps\SOIL2\" >nul
    )
    echo [SOIL2] Done. Rebuild with _GENERATE_MINGW_x64.bat to compile it.
)
:next5

rem ============================================================
rmdir /s /q "%tmpDir%" 2>nul

echo.
echo ============================================================
echo  Update finished. If you updated MinGW or CMake, open a NEW
echo  terminal, run _SET_PATHS_x64.bat, then rebuild everything
echo  with glfw-master\_GENERATE_MINGW_x64.bat
echo ============================================================

:end
endlocal
pause
