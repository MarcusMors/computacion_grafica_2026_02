# ADR-0001: One source tree, one bundle per OS, compilers per OS

- **Status:** accepted
- **Date:** 2026-10-25
- **Deciders:** Marcus
- **Comes from:** the planning conversation (see [PLAN.md](../PLAN.md) sections 2, 3, 6)

## Context

LabCompiler exists so students can compile on lab machines that have no compiler and restrictive permissions (Windows).
Linux machines have `sudo`; macOS users install their own tools. The goal is learning and readability, not a single
binary for every platform.

## Decision

- One source tree. Different **script files** per OS: `.bat` / `.ps1` on Windows, `.sh` on Linux and macOS.
- Binaries are built on the user's own machine and only have to work there.
- **Windows:** keep the bundled portable compiler (WinLibs MinGW GCC 16.2), CMake and editor.
- **Linux:** use the system compiler when it passes the `#embed` probe; otherwise install GCC 15+ (apt, or the PPA) or,
  optionally, download xPack GCC 15.2.0-1 into the folder. Binaries link `-static-libstdc++ -static-libgcc`.
- **macOS:** Homebrew `gcc` or `llvm` (Apple Clang 17 has no `#embed`). Best effort until a friend tests it.
- CMake: provided through Kitware's official binaries (all three OSes) as an optional download; a system CMake >= 3.20 also works.

## Consequences

- (+) Windows lab machines keep working with no installation.
- (-) On Linux and macOS the first setup needs `sudo` / Homebrew.
- Needs: `_CHECK_ENV.sh` with the `#embed` probe and exact install commands; `_UPDATE_COMPONENTS.sh` (CMake, optional xPack).
