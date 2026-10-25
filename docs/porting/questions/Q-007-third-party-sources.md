# Q-007: How does a fresh clone get GLFW and the other third-party sources?

- **Topic:** porting
- **Status:** decided (-> ADR-0005)
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, **before the first push** (it decides what the repository contains)

## Context
`.gitignore` excludes `glfw-master/src`, `include`, `CMake`, `docs`, `examples`, `tests` and `deps/*` (except `deps/glad`). Anyone
who already has the LabCompiler folder is fine, but a fresh `git clone` cannot build, and neither can GitHub Actions (which would
give us macOS and Linux test runs for free). Sizes: those ignored folders total about 9 MB; Boost (191 MB), the compilers and
CMake are far bigger and stay out.

## Options
### A. Track GLFW and `deps` in git (about 9 MB)
- (+) `git clone` builds right away; CI is trivial.
- (-) Third-party code in our history (but it never changes).

### B. A bootstrap script downloads pinned releases (with checksums)
- (+) Repository stays tiny.
- (-) Needs network at setup; a script to maintain; CI needs it too.

### C. Git submodule or CMake FetchContent for GLFW
- (+) Standard tooling.
- (-) Our `deps` (glad patch, glm, SOIL2, stb) still need a home; more concepts for students.

### D. Keep third-party code and tools OUT of git; the LabCompiler zip is the common base
Everybody who works on the repository already has the full LabCompiler folder (GLFW, `deps`, compilers, Boost) from the zip.
- (+) The repository holds only what concerns everyone: `my_lib`, the projects in `OwnProjects`, scripts and docs.
- (-) A bare `git clone` cannot build, and GitHub Actions cannot build it without downloading those pieces first.

## Recommendation
A. 9 MB is small, and it makes a clone self-sufficient. This reverses part of the earlier "do not track libraries" choice,
which was about the heavy bundles (compilers, Boost), not these.

## Answers
- Marcus: **D**. My partners and I already have exactly the same folder from the zip, and we do not touch those libraries, so they do not belong on GitHub.

## Decision

D -> [ADR-0005](../adr/0005-repository-holds-only-our-code.md).
