@echo off
rem Run git on a drive that does not record file ownership (USB stick, FAT/exFAT),
rem WITHOUT changing any git setting of this computer: the "safe.directory" exception
rem is given to this one command only.   Use it like git:   _GIT status   /   _GIT add -A   /   _GIT commit -m "msg"
setlocal
set GIT_CONFIG_COUNT=1
set GIT_CONFIG_KEY_0=safe.directory
set GIT_CONFIG_VALUE_0=*
git -C "%~dp0." %*
