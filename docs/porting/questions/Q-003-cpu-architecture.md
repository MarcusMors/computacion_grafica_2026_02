# Q-003: Do the scripts support ARM (Apple Silicon, Linux arm64) from the start?

- **Topic:** porting
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus

## Context
Downloads (CMake, xPack GCC) differ per CPU. Apple Silicon Macs and ARM Linux laptops exist in the class.

## Options
### A. Detect with `uname -m` and pick the right download (x86_64 / arm64)
- (+) Works for everybody.
- (-) A few more lines per download, and untested on ARM until someone tries.

### B. x86_64 only for now
- (+) Less to test.
- (-) Apple Silicon friends cannot use the downloads.

## Recommendation
A (macOS has a universal CMake build; only Linux needs the switch).

## Answers
- Marcus:

## Decision
