# Control_1 — Rationale of the Changes

This document explains **what** changed in the project and **why**, grouped by
the three goals: (1) a seamless uniform-passing API, (2) a 3D pyramid in
`main.cpp`, (3) a camera class.

---

## 1. The problem with uniforms as they were

Passing one `float` to a shader required **three pieces of code in three
different places**, plus a global variable per uniform:

```cpp
// globals
GLint scaleLoc_interior;                                   // 1. storage

// init()
scaleLoc_interior = glGetUniformLocation(program_interior_ptr->id, "u_scale"); // 2. lookup

// display()
program_interior_ptr->use();                               // 3a. bind
glUniform1f(scaleLoc_interior, scale);                     // 3b. upload
```

Costs of that pattern:

- **Global `GLint` variables** proliferate (one per uniform per program).
- The lookup and the upload are **far apart**, so it's easy to query a
  location from program A and upload it to program B.
- `glUniform*` silently targets **whatever program happens to be bound** —
  forgetting `use()` is a classic source of "my uniform does nothing" bugs.
- `glGetUniformLocation` is a GL call; the natural fix (call it every frame)
  is wasteful, so people hand-roll caches anyway.

## 2. The new uniform API (`_cpp.hpp` / `_cpp.cpp`)

`Program` now owns the whole mechanism. The four lines above become one:

```cpp
program_ptr->set_uniform("u_scale", scale);
```

Design decisions:

- **`set_uniform()` calls `use()` for you.** A uniform can never land on the
  wrong program, and the call site no longer needs to remember to bind first.
- **Locations are cached per `Program` instance**
  (`mutable std::unordered_map<std::string, GLint>`). The first call with a
  given name does the `glGetUniformLocation`; every later call is a hash
  lookup. The cache is `mutable` so the whole API stays `const`, exactly like
  `use()` — filling a cache doesn't change the observable state.
- **A bad name warns once, not every frame.** `glGetUniformLocation` returns
  `-1` for a typo'd or optimized-out uniform; `glUniform*(-1, ...)` is defined
  by the spec as a silent no-op, so the program degrades gracefully instead of
  crashing, and you get a single console warning telling you which name failed.
- **Overloads cover the everyday types**: `float`, `int`, `unsigned`, `bool`,
  `(x,y)` / `(x,y,z)` / `(x,y,z,w)` component lists, the project's own
  `Vec2`/`Vec3`/`Vec4` from `Buffers.hpp`, and `Mat4` (below). Adding more
  (e.g. `int` vectors, arrays) is a one-line overload following the same shape.
- **`Mat4` is only forward-declared in `_cpp.hpp`** (`struct Mat4;`) and the
  matrix overload is defined in `_cpp.cpp`, where `Mat4.hpp` is included. This
  keeps the matrix header out of `_cpp.hpp`'s include list — files that don't
  do 3D don't pay for it — while callers, who include `Mat4.hpp` anyway, see a
  complete type.
- **Fully backward compatible.** `Program::id` is still public, so the old
  manual `glGetUniformLocation(program.id, ...)` style keeps working; nothing
  existing had to be rewritten.

How the pizza code would look migrated (for reference — the pizza itself was
replaced per the task):

```cpp
program_interior_ptr->set_uniform("u_scale", scale);   // was: use() + glUniform1f(scaleLoc_interior, scale)
```

## 3. `Mat4.hpp` (new) — just enough 3D math

The camera and the pyramid need 4×4 matrices, but the project has no math
library. Instead of adding a glm dependency, `Mat4.hpp` provides the handful
of operations actually used:

- `struct Mat4 { float m[16]; }` — **column-major**, `m[col*4 + row]`, the same
  convention as OpenGL and glm. That is why `set_uniform(name, mat4)` can pass
  `GL_FALSE` (no transpose) to `glUniformMatrix4fv` and hand over
  `matrix.data()` directly.
- Free operators on the existing `Vec3` (`+ - * dot cross normalize length`) —
  `Vec3` itself, defined in `Buffers.hpp`, is untouched.
- `mat4_perspective`, `mat4_look_at`, `mat4_translate`, `mat4_rotate`
  (Rodrigues' formula), `mat4_scale`, and `operator*` with the usual
  right-to-left application order (`P * V * M` applies `M` first), matching
  GLSL/glm reading order.

All four transforms were verified numerically against reference formulas
(e.g. a point at `z = -near` maps to clip `z = -1`; `look_at` from `(0,0,3)`
sends the origin to view-space `(0,0,-3)`; a 90° rotation about Y sends +X to
−Z).

## 4. `Camera.hpp` (new) — what a camera class should own

The idea: a camera is **state + two matrices**. Everything else is convenience.

- **State**: `position`, `yaw_deg`/`pitch_deg` (Euler angles, stored in
  degrees because they're tuned by hand), plus lens settings (`fov_y_deg`,
  `near_plane`, `far_plane`). `front()` derives the look direction from the
  angles; pitch is clamped to ±89° so the direction never degenerates at the
  poles.
- **The two matrices**: `view()` = `look_at(position, position + front(), up)`
  and `projection(aspect)` = perspective matrix. `aspect` is a parameter (not
  stored) because it changes with the window size and is recomputed every frame.
- **`process_keyboard(window, dt)`** is deliberately *optional*: WASD + E/Q
  movement and arrow-key looking, scaled by the frame time `dt` so speed is
  frame-rate independent. It's a convenience, not part of the contract — swap
  in mouse-look later without touching `view()`/`projection()`.

## 5. `main.cpp` — the 3D pyramid

- **Star experiment removed** (`program_star`, `vertex_star.glsl`,
  `fragment_star.glsl`, and the unused `vertices_star` block), as requested.
- **Geometry**: square base on the `y = -0.5` plane (half side 0.5), apex at
  `(0, 0.5, 0)` — **5 vertices, 18 indices (6 triangles)**. Unlike the pizza's
  independent triangles, the base corners are shared by three triangles each,
  so this uses the **indexed path** of the library for the first time:
  `Index_Buffer<uint32_t>` + `Vertex_Array::attach_indices()` +
  `draw_elements()`. The index type (`GL_UNSIGNED_INT`) is deduced from the
  template parameter, never written by hand. Winding is counter-clockwise seen
  from outside, so enabling `GL_CULL_FACE` later is safe.
- **Per-vertex color** (`struct Vertex { Vec3 pos; Vec3 color; }`): each base
  corner gets a distinct color and the apex is white; the rasterizer
  interpolates them, so the faces stay readable with **no lighting code**.
- **3D requires new per-frame state**:
  - `glEnable(GL_DEPTH_TEST)` + clearing `GL_DEPTH_BUFFER_BIT` — without it,
    draw order decides visibility instead of distance.
  - `glViewport` + aspect ratio recomputed every frame from the framebuffer
    size, so resizing the window never stretches the image.
  - A real **frame time `dt`** for the camera (and an `ESC`-to-quit key).
- **Uniforms in the new style** — the point of the exercise:

  ```cpp
  const Mat4 mvp = camera_ptr->projection(aspect) * camera_ptr->view() * model;
  program_ptr->set_uniform("u_mvp", mvp);      // a Mat4 uniform
  program_ptr->set_uniform("u_pulse", pulse);  // a float uniform, same one-liner
  ```

  `model` spins the pyramid around Y (one turn per 4 s); `u_pulse` gently
  modulates brightness purely to show a second uniform type going through the
  same API. No global `GLint`s, no `glGetUniformLocation` in `init()`.

- **Kept as-is**: the `init()` / `display()` / `main()` skeleton, the
  `LOGGING.hpp` guards, `#embed` shader embedding, and the raw-GLFW window
  setup — the changes concentrate on what the task asked for.

## 6. Shaders

| File | Change |
|---|---|
| `vertex.glsl` | Now 3D: `in vec3 a_pos` (location 0), `in vec3 a_color` (location 1), `uniform mat4 u_mvp`; passes `v_color` through. |
| `fragment.glsl` | Outputs `vec4(v_color * u_pulse, 1.0)` — interpolated color times the float uniform. |
| `fragment_interior.glsl`, `fragment_border.glsl`, `fragment_red.glsl` | **Untouched.** They belonged to the pizza; the new `main.cpp` doesn't reference them, but they still compile and remain available. |

## 7. Build notes

- New **header-only** files: `Mat4.hpp`, `Camera.hpp` — nothing to add to the
  linker; just make sure they're in the include path (same folder as the other
  headers).
- `_cpp.cpp` is still the only library translation unit; `_cpp.hpp` gained
  `#include <unordered_map>` and `#include "Buffers.hpp"` (for the `Vec*`
  overloads) — no new external dependencies.
- Requires C++20 or later, as before (designated initializers, `#embed`,
  `std::numbers`).

## 8. Verification performed

- `Mat4` perspective / look-at / rotate / multiply checked numerically against
  reference formulas (Python) and as compiled C++ — exact match.
- All modified/new C++ compiled with `g++ -std=c++20 -Wall -Wextra` against
  stub GL headers (the sandbox has no GPU/GLFW): **zero errors**; the only
  warnings are pre-existing ones from the original code (`Shader::constructor`'s
  unused `t_type`, `init()`'s unused `window`).
