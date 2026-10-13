@echo off
rem ============================================================
rem  LabCompiler - environment bootstrap
rem  Run once per CMD session from the project root.
rem ============================================================

rem Backup the current PATH to a file
echo Backing up the current PATH...
echo %path%>path_backup.txt

rem Store the base directory
set baseDir=%cd%

rem Display the current file path for the compiler and CMake
echo The current file path will use to our compiler and cmake is:
echo %baseDir%

rem Auto-detect the compiler folder (first folder inside MinGW\)
set compilerDir=
for /d %%D in ("%baseDir%\MinGW\*") do (
    if not defined compilerDir set compilerDir=%%D\bin
)

rem Auto-detect the CMake folder (first folder inside Cmake\)
set cmakeDir=
for /d %%D in ("%baseDir%\Cmake\*") do (
    if not defined cmakeDir set cmakeDir=%%D\bin
)

if not defined compilerDir (
    echo ERROR: no compiler found under %baseDir%\MinGW\
    goto :eof
)
if not defined cmakeDir (
    echo ERROR: no CMake found under %baseDir%\Cmake\
    goto :eof
)

rem Display the defined paths
echo "compilerDir = " %compilerDir%
echo "cmakeDir = " %cmakeDir%

rem Update the PATH environment variable
echo Updating the PATH environment variable...
set path=%compilerDir%;%PATH%
set path=%cmakeDir%;%PATH%
