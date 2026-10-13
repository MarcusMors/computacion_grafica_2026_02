<#
  LabCompiler - snapshot / status of the project files (a "git status" that needs no git).

  Usage (from the LabCompiler root, in CMD or PowerShell):
      _SNAPSHOT.bat save [name]       take a snapshot of every project file   (default name: baseline)
      _SNAPSHOT.bat status [name]     compare the files NOW against that snapshot   (the default command)
      _SNAPSHOT.bat list              show the snapshots that exist
      add -Full to status to re-hash every file (normally a file is only re-read when
      its size or modification time changed)

  What it reports:
      ADDED      file exists now, was not in the snapshot
      MODIFIED   content changed (SHA-256 differs)
      TOUCHED    modification time changed but the content is identical
      DELETED    file was in the snapshot, is gone now

  Not tracked (big, or generated): MinGW\, Cmake\, Editor\, Boost\, .git, .vscode, .claude, build folders,
  CMakeFiles, compiled files (*.exe *.o *.obj *.a *.lib *.dll *.out) and the snapshots themselves.
  Edit $SkipRootDirs / $SkipDirNames / $SkipExtensions below to change that.

  Snapshots are plain text (tab separated) in _snapshots\<name>.tsv:  path, size, mtime ticks, sha256
#>
param(
  [string]$Command = 'status',
  [string]$Name = 'baseline',
  [switch]$Full
)

$ErrorActionPreference = 'Stop'
$Root = $PSScriptRoot
$SnapDir = Join-Path $Root '_snapshots'

# ---- what to leave out --------------------------------------------------------------------
$SkipRootDirs  = @('MinGW', 'Cmake', 'Editor', 'Boost', '_snapshots')                       # only at the project root
$SkipDirNames  = @('.git', '.vscode', '.claude', '.idea', 'build', 'mak.mingw.x64', 'CMakeFiles')  # at any depth
$SkipExtensions = @('.exe', '.o', '.obj', '.a', '.lib', '.dll', '.out')
$SkipFiles     = @('path_backup.txt')                                              # only at the project root

# ---- walk the tree (skipping the big folders WITHOUT entering them) -----------------------
function Get-ProjectFiles {
  $stack = New-Object System.Collections.Generic.Stack[string]
  $stack.Push($Root)
  while ($stack.Count -gt 0) {
    $dir = $stack.Pop()
    $isRoot = ($dir -eq $Root)
    foreach ($d in [System.IO.Directory]::GetDirectories($dir)) {
      $leaf = [System.IO.Path]::GetFileName($d)
      if ($SkipDirNames -contains $leaf) { continue }
      if ($isRoot -and ($SkipRootDirs -contains $leaf)) { continue }
      $stack.Push($d)
    }
    foreach ($f in [System.IO.Directory]::GetFiles($dir)) {
      $leaf = [System.IO.Path]::GetFileName($f)
      if ($SkipExtensions -contains [System.IO.Path]::GetExtension($f).ToLowerInvariant()) { continue }
      if ($isRoot -and ($SkipFiles -contains $leaf)) { continue }
      $f
    }
  }
}

function Get-Sha256([string]$path) {
  $sha = [System.Security.Cryptography.SHA256]::Create()
  try {
    $stream = [System.IO.File]::Open($path, 'Open', 'Read', 'ReadWrite')
    try { return ([BitConverter]::ToString($sha.ComputeHash($stream)) -replace '-', '').ToLowerInvariant() }
    finally { $stream.Dispose() }
  } catch { return 'UNREADABLE' }
  finally { $sha.Dispose() }
}

function Get-RelPath([string]$full) { return $full.Substring($Root.Length).TrimStart('\').Replace('\', '/') }

# ---- read a snapshot file -> hashtable  path -> @{Size; Ticks; Hash} ------------------------
function Read-Snapshot([string]$file) {
  $map = @{}
  $created = ''
  foreach ($line in [System.IO.File]::ReadAllLines($file)) {
    if ($line.StartsWith('#')) { if ($line.StartsWith('# created ')) { $created = $line.Substring(10) }; continue }
    $p = $line.Split("`t")
    if ($p.Count -ne 4) { continue }
    $map[$p[0]] = @{ Size = [int64]$p[1]; Ticks = [int64]$p[2]; Hash = $p[3] }
  }
  return @{ Files = $map; Created = $created }
}

# ---- scan the files now. $old (optional) lets us skip re-hashing unchanged files ------------
function Scan-Now($old) {
  $now = @{}
  foreach ($f in Get-ProjectFiles) {
    $fi = New-Object System.IO.FileInfo $f
    $rel = Get-RelPath $f
    $size = $fi.Length
    $ticks = $fi.LastWriteTimeUtc.Ticks
    if ($old -and -not $Full -and $old.ContainsKey($rel) -and $old[$rel].Size -eq $size -and $old[$rel].Ticks -eq $ticks) {
      $hash = $old[$rel].Hash          # same size + same timestamp: trust the stored hash
    } else {
      $hash = Get-Sha256 $f
    }
    $now[$rel] = @{ Size = $size; Ticks = $ticks; Hash = $hash }
  }
  return $now
}

switch ($Command.ToLowerInvariant()) {

  'save' {
    $now = Scan-Now $null
    if (-not (Test-Path $SnapDir)) { New-Item -ItemType Directory -Path $SnapDir | Out-Null }
    $lines = New-Object System.Collections.Generic.List[string]
    $lines.Add('# LabCompiler snapshot')
    $lines.Add('# created ' + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss') + ' on ' + $env:COMPUTERNAME)
    foreach ($k in ($now.Keys | Sort-Object)) {
      $v = $now[$k]
      $lines.Add("$k`t$($v.Size)`t$($v.Ticks)`t$($v.Hash)")
    }
    $out = Join-Path $SnapDir "$Name.tsv"
    [System.IO.File]::WriteAllLines($out, $lines, (New-Object System.Text.UTF8Encoding($false)))
    Write-Host "Snapshot '$Name' saved: $($now.Count) files  ->  _snapshots\$Name.tsv" -ForegroundColor Green
  }

  'list' {
    if (-not (Test-Path $SnapDir)) { Write-Host 'No snapshots yet. Run:  _SNAPSHOT.bat save'; break }
    foreach ($f in Get-ChildItem $SnapDir -Filter *.tsv) {
      $s = Read-Snapshot $f.FullName
      '{0,-20} {1,6} files   created {2}' -f $f.BaseName, $s.Files.Count, $s.Created
    }
  }

  'status' {
    $file = Join-Path $SnapDir "$Name.tsv"
    if (-not (Test-Path $file)) {
      Write-Host "No snapshot named '$Name'. Take one first:  _SNAPSHOT.bat save $Name" -ForegroundColor Yellow
      exit 1
    }
    $snap = Read-Snapshot $file
    $old = $snap.Files
    $now = Scan-Now $old

    $added = @(); $modified = @(); $touched = @(); $deleted = @()
    foreach ($k in ($now.Keys | Sort-Object)) {
      if (-not $old.ContainsKey($k)) { $added += $k; continue }
      if ($now[$k].Hash -ne $old[$k].Hash) { $modified += $k }
      elseif ($now[$k].Ticks -ne $old[$k].Ticks) { $touched += $k }
    }
    foreach ($k in ($old.Keys | Sort-Object)) { if (-not $now.ContainsKey($k)) { $deleted += $k } }

    Write-Host "Compared with snapshot '$Name' (created $($snap.Created))"
    Write-Host ''
    foreach ($k in $added)    { Write-Host ("  ADDED     " + $k) -ForegroundColor Green }
    foreach ($k in $modified) {
      Write-Host ("  MODIFIED  " + $k + "   ($($old[$k].Size) -> $($now[$k].Size) bytes, sha256 " +
                  $old[$k].Hash.Substring(0, 8) + ' -> ' + $now[$k].Hash.Substring(0, 8) + ')') -ForegroundColor Yellow
    }
    foreach ($k in $touched)  { Write-Host ("  TOUCHED   " + $k + "   (timestamp only, content identical)") -ForegroundColor DarkGray }
    foreach ($k in $deleted)  { Write-Host ("  DELETED   " + $k) -ForegroundColor Red }
    if (($added.Count + $modified.Count + $touched.Count + $deleted.Count) -eq 0) {
      Write-Host '  Nothing changed.' -ForegroundColor Green
    }
    Write-Host ''
    Write-Host ("{0} added, {1} modified, {2} touched, {3} deleted   ({4} files tracked now)" -f
                $added.Count, $modified.Count, $touched.Count, $deleted.Count, $now.Count)
  }

  default {
    Write-Host "Unknown command '$Command'. Use: save [name] | status [name] | list   (add -Full to status to re-hash everything)"
    exit 2
  }
}
