# Q-005: Do we port `_SNAPSHOT` and `_GIT` to shell scripts now?

- **Topic:** porting
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus

## Context
`_SNAPSHOT.ps1` (hash-based "what changed" report) and `_GIT.bat` (git with a safe.directory exception on USB drives) exist
for the Windows labs. Linux and macOS machines normally have a working git.

## Options
### A. Later, only if someone asks
- (+) Focus on the build scripts.
- (-) Linux users have no snapshot tool.

### B. Now: `_SNAPSHOT.sh` with `sha256sum` / `shasum`, `_GIT.sh`
- (+) Feature parity.
- (-) More scripts to test on three systems.

## Recommendation
A.

## Answers
- Marcus:

## Decision
