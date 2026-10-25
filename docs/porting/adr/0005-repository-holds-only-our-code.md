# ADR-0005: The repository holds only our code; the LabCompiler zip is the common base

- **Status:** accepted
- **Date:** 2026-10-25
- **Deciders:** Marcus
- **Comes from:** [Q-007](../questions/Q-007-third-party-sources.md)

## Context

LabCompiler is a big folder (GLFW and its dependencies, glad, glm, SOIL2, compilers, CMake, an editor, Boost). The people working on
the course project all received the same folder as a zip and never edit those libraries.

## Decision

- Git tracks only what concerns the team and the students: `my_lib/`, the projects in `glfw-master/OwnProjects/`,
  `glfw-master/CMakeLists.txt`, the setup / build scripts, the docs and the project configuration files.
- Third-party code and tools stay out of git (`.gitignore`): `glfw-master/src`, `include`, `CMake`, `docs`, `examples`, `tests`,
  `deps/*` (except the patched `deps/glad`), the compilers, CMake, the editor and Boost.
- The **LabCompiler zip** is the shared base. Whoever joins gets the zip first, then attaches it to the repository (see `docs/README.md`).

## Consequences

- (+) A small repository; no heavy files in history.
- (-) A bare `git clone` cannot build; GitHub Actions would need a step that downloads the missing pieces (not planned now),
  so macOS and Linux testing relies on people running the build themselves.
- When a third-party component changes (new GLFW, new glad patch) the zip must be re-issued and the change announced.
