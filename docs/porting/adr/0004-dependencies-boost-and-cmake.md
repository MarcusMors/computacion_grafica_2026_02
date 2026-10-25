# ADR-0004: Boost and CMake on each OS

- **Status:** accepted
- **Date:** 2026-10-25
- **Deciders:** Marcus
- **Comes from:** [PLAN.md](../PLAN.md) section 6

## Context

The look-at viewer uses `boost::rational`. On Windows the full Boost headers (1.92.0, 158 MB, no compiled libraries) live
in `LabCompiler/Boost/`, which is not in git. Official CMake binaries exist for Windows, Linux (x86_64, aarch64) and
macOS (universal): 4.4.3 is 65 / 52 / 89 MB.

## Decision

- Windows keeps the bundled `Boost/` folder.
- Linux and macOS use the **system package** (`libboost-dev`, `brew install boost`); `my_lib/CMakeLists.txt` already
  prefers `LabCompiler/Boost/` and falls back to the system.
- CMake is **provided** (optional download in `_UPDATE_COMPONENTS.sh` from Kitware's releases, like the Windows script);
  if it is absent a system CMake >= 3.20 is required.

## Consequences

- (+) No 191 MB Boost copy per OS; same CMake version everywhere when the user wants it.
- (-) One more step in `_CHECK_ENV.sh` (Boost headers present?).
