# ADR-0003: OpenGL version policy

- **Status:** accepted
- **Date:** 2026-10-25
- **Deciders:** Marcus
- **Comes from:** [PLAN.md](../PLAN.md) section 4

## Context

macOS supports OpenGL 4.1 at most (core, forward-compatible only). Today `my_lib` asks for 4.3 and all shaders say
`#version 430`. A scan found nothing in the repository that needs more than 4.1 (no compute, SSBO, image load/store,
`layout(binding)`, explicit uniform locations, debug output).

## Decision

1. Shaders inside `my_lib` (`Renderer`, `Draw2D`) become `#version 410 core`.
2. The lab projects' shaders are **edited** from `#version 430` to `#version 410` (no run-time rewriting).
3. `request_opengl_version()` keeps asking for **4.3** by default on Windows and Linux ("let each student explore their
   hardware"), with optional `(major, minor)` arguments. On macOS it applies the only context that works (4.1 core +
   forward-compat) and prints one notice line. `max_opengl_supported` stays as the exploration tool.
4. A lab that really needs 4.2+ features asks for them explicitly and is documented as "not on macOS".

## Consequences

- (+) The same shader text runs on all three OSes.
- (-) Students lose nothing today; compute shaders and friends stay unavailable on macOS.
- macOS: `glLineWidth(2)` is clamped to 1 (cosmetic); Project_00 requests a 3.x context and needs forward-compat there.
