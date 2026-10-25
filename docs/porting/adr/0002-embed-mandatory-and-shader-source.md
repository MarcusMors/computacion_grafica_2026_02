# ADR-0002: `#embed` is required; `Shader_Source` also accepts a path or a literal

- **Status:** accepted
- **Date:** 2026-10-25
- **Deciders:** Marcus
- **Comes from:** [PLAN.md](../PLAN.md) sections 3 and 5.3

## Context

The projects write `#embed "vertex.glsl"`. GCC 14 (Ubuntu 24.04) rejects it; GCC 15+ and Clang 19+ accept it. Apple Clang does not.

## Decision

- `#embed` support is **mandatory** for the projects that use it. If a machine cannot compile it, that is the user's
  problem: the setup script says so clearly and how to fix it.
- Escape hatch for such users: paste the shader text into a string literal, **or** pass a file path.
  `Shader_Source(std::string_view)` decides:
  - no newline, `{` or `;`, short, and a shader extension (`.glsl .vert .frag .vs .fs .geom .tesc .tese .comp`)
    -> it is a **path**: read the file (current folder, then the executable's folder);
  - a path-looking string whose file is not found **throws** with the path and the current folder;
  - anything else is a literal.
- CMake runs an `#embed` probe at configure time and stops with a message pointing to `_CHECK_ENV.sh`.

## Consequences

- (+) Students on older compilers still have a way to run every project.
- (-) A tiny heuristic in `Shader_Source`; it must be documented in its header.
