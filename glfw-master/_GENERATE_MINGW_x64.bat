@echo off

set currentDir=%cd%

if exist build\ (
	echo build folder exist, if you are using the USB for first time on this PC, this should be deleted!!
	rmdir /s build
)

echo Creating a new build folder...

mkdir build
chdir build

mkdir mak.mingw.x64
chdir mak.mingw.x64

echo Running CMake to generate Makefiles...
cmake -G "MinGW Makefiles" ../../
if %errorlevel% NEQ 0 goto :fail_cmake

echo Compiling with MinGW...
mingw32-make
if %errorlevel% NEQ 0 goto :fail_make

echo Success compilation !! - press key to continue.
cd %currentDir%\bin
goto :eof

:fail_cmake
echo !! CMake generation failed - check that _SET_PATHS_x64 was run first.
echo !! (in CMD run: _SET_PATHS_x64.bat   -   in PowerShell:  . .\_SET_PATHS_x64.ps1 )
pause >nul
goto :eof

:fail_make
echo !! Compilation failed - scroll up to see the compiler errors.
pause >nul
goto :eof
