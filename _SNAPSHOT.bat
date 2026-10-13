@echo off
rem Snapshot / status of the project files without git. See _SNAPSHOT.ps1 for details.
rem   _SNAPSHOT save [name]   |   _SNAPSHOT status [name]   |   _SNAPSHOT list
powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0_SNAPSHOT.ps1" %*
