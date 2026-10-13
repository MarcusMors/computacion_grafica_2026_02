# ============================================================
#  LabCompiler - environment bootstrap (PowerShell version)
#
#  IMPORTANT: dot-source it so the PATH changes persist in
#  THIS PowerShell session:
#
#      . .\_SET_PATHS_x64.ps1
#
#  (note the leading ". " - running it without the dot executes
#  it in a child scope and the PATH changes would be lost again)
#
#  If PowerShell blocks the script (execution policy), run once:
#      Set-ExecutionPolicy -Scope CurrentUser RemoteSigned
# ============================================================

$baseDir = Get-Location

"Backing up the current PATH to path_backup.txt ..."
$env:PATH | Out-File -Encoding ascii "$baseDir\path_backup.txt"

"The current file path will use to our compiler and cmake is:"
"$baseDir"

# Auto-detect the compiler folder (first folder inside MinGW\)
$compilerDir = Get-ChildItem -Directory "$baseDir\MinGW" -ErrorAction SilentlyContinue | Select-Object -First 1
# Auto-detect the CMake folder (first folder inside Cmake\)
$cmakeDir    = Get-ChildItem -Directory "$baseDir\Cmake" -ErrorAction SilentlyContinue | Select-Object -First 1

if (-not $compilerDir) { Write-Error "no compiler found under $baseDir\MinGW\"; return }
if (-not $cmakeDir)    { Write-Error "no CMake found under $baseDir\Cmake\"; return }

$compilerBin = Join-Path $compilerDir.FullName "bin"
$cmakeBin    = Join-Path $cmakeDir.FullName "bin"

"compilerDir = $compilerBin"
"cmakeDir    = $cmakeBin"

"Updating the PATH environment variable (this session only)..."
$env:PATH = "$cmakeBin;$compilerBin;$env:PATH"

""
"Done. Verify with:  g++ --version   and   cmake --version"
