# Q-002: How does a Linux user get a compiler that accepts `#embed`?

- **Topic:** porting
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus

## Context
`#embed` needs GCC 15+ (ADR-0002). Ubuntu 26.04 ships GCC 15.2; Ubuntu 22.04 / 24.04 stop at GCC 12 / 14 (a PPA gives newer).
xPack GCC 15.2.0-1 is a portable download for Linux x64 and arm64 (glibc >= 2.28). It still needs the system C headers and the
X11 / Wayland / GL development packages, which `sudo apt` installs.

## Options
### A. `_CHECK_ENV.sh` only
It runs the `#embed` probe and prints the exact `apt` / PPA commands.
- (+) Simplest; nothing downloaded by our scripts.
- (-) PPAs are an extra step on older Ubuntu.

### B. A, plus an optional `_UPDATE_COMPONENTS.sh` toggle that downloads xPack GCC 15.2.0-1 into the folder
Checksum verified; used by `_SET_PATHS.sh` when present.
- (+) Same compiler version on every distro; no PPA.
- (-) ~150 MB download; a version and a URL to keep up to date.

## Recommendation
B, with xPack optional.

## Answers
- Marcus:

## Decision
