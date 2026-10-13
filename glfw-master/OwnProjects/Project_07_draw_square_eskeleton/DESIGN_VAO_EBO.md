# Design Notes — VAO / EBO and the drawing API of `my_lib`

Status: **proposal — awaiting your approval / edits.**
Scope: `my_lib/_cpp.hpp`, `my_lib/_cpp.cpp`, `my_lib/Buffers.hpp`, used by
`glfw-master/OwnProjects/Project_07_draw_square_eskeleton`.

---

## 1. Audit of the current state

### 1.1 The `GL_Draw` enum fix — verified correct ✓

Checked every enumerator against `glfw-master/deps/glad/gl.h`:

| `GL_Draw` enumerator | Enum value (fixed) | Real GL constant | Value in `gl.h` | OK |
|---|---|---|---|---|
| `POINTS` | `GL_POINTS` | 0x0000 | 0x0000 | ✓ |
| `LINES` | `GL_LINES` | 0x0001 | 0x0001 | ✓ |
| `LINE_LOOP` | `GL_LINE_LOOP` | 0x0002 | 0x0002 | ✓ |
| `LINE_STRIP` | `GL_LINE_STRIP` | 0x0003 | 0x0003 | ✓ |
| `TRIANGLES` | `GL_TRIANGLES` | 0x0004 | 0x0004 | ✓ |
| `TRIANGLE_STRIP` | `GL_TRIANGLE_STRIP` | 0x0005 | 0x0005 | ✓ |
| `TRIANGLE_FAN` | `GL_TRIANGLE_FAN` | 0x0006 | 0x0006 | ✓ |
| `LINES_ADJACENCY` | `GL_LINES_ADJACENCY` | 0x000A | 0x000A | ✓ |
| `LINE_STRIP_ADJACENCY` | `GL_LINE_STRIP_ADJACENCY` | 0x000B | 0x000B | ✓ |
| `TRIANGLES_ADJACENCY` | `GL_TRIANGLES_ADJACENCY` | 0x000C | 0x000C | ✓ |
| `TRIANGLE_STRIP_ADJACENCY` | `GL_TRIANGLE_STRIP_ADJACENCY` | 0x000D | 0x000D | ✓ |

The fix is the right pattern: **always initialize from the `GL_*` constant, never
rely on implicit 0,1,2,… ordering.** GL primitive constants are not sequential.

**One gap found:** `GL_PATCHES` (0x000E) is missing. It matters because your
`Shader_Pipeline` already supports tessellation stages, and tessellation
*requires* drawing with `GL_PATCHES` (plus `glPatchParameteri(GL_PATCH_VERTICES, n)`).
Recommend adding:

```cpp
PATCHES = GL_PATCHES,  // requires tessellation shaders + glPatchParameteri
```

### 1.2 Which modes work with which draw call

Both functions accept the **same** set of primitive modes — the mode is about
*how vertices are assembled*, not about *where they come from*.

| Mode | `glDrawArrays_cpp` | `glDrawElements_cpp` | Note |
|---|---|---|---|
| `POINTS` | ✓ | ✓ | |
| `LINES`, `LINE_STRIP`, `LINE_LOOP` | ✓ | ✓ | |
| `TRIANGLES`, `TRIANGLE_STRIP`, `TRIANGLE_FAN` | ✓ | ✓ | |
| `*_ADJACENCY` (all 4) | ✓ | ✓ | Only useful with a **geometry shader**; without one the extra vertices are silently dropped |
| `PATCHES` (proposed) | ✓ | ✓ | **Only** valid with tessellation shaders; also needs `glPatchParameteri` |

So: **all `GL_Draw` values are compatible with both functions.** The real
difference between the two calls is the data source:

- `glDrawArrays(mode, first, count)` — "draw vertices `first … first+count-1`,
  in storage order." Needs only a VAO (your square works with no VBO at all
  because the shader generates positions from `gl_VertexID`).
- `glDrawElements(mode, count, type, indices)` — "draw `count` *indices* read
  from the bound EBO; each index selects a vertex from the VBOs." Needs
  **VAO + VBO + EBO**. Without an EBO bound in the current VAO the call is
  invalid (undefined behavior / crash).

---

## 2. Object glossary (one line each, for orientation)

| Object | What it actually is |
|---|---|
| **VAO** (Vertex Array Object) | A *container of state*, not data. Remembers: attribute layouts, which VBO feeds each attribute, and **which EBO is bound**. |
| **VBO** (Vertex Buffer Object) | Raw GPU memory with vertex data (positions, colors, …). `GL_ARRAY_BUFFER`. |
| **EBO / IBO** (Element/Index Buffer) | Raw GPU memory with *indices* (integers) into the VBOs. `GL_ELEMENT_ARRAY_BUFFER`. Same thing, two names. |
| **UBO** (Uniform Buffer) | `GL_UNIFORM_BUFFER` — uniform blocks shared across shaders. |
| **SSBO** (Shader Storage Buffer) | `GL_SHADER_STORAGE_BUFFER` — large read/write data for shaders (compute). |
| **TBO** (Texture Buffer) | `GL_TEXTURE_BUFFER` — a buffer viewed as a 1-D texture. |
| **PBO** (Pixel Buffer) | `GL_PIXEL_(UN)PACK_BUFFER` — async CPU↔GPU pixel transfers. |
| **FBO** (Framebuffer Object) | Off-screen render target — a set of attachments, **not** a buffer of bytes. |
| **RBO** (Renderbuffer Object) | An FBO attachment you can't sample as a texture (typical: depth/stencil). |

Only VAO, VBO and EBO matter for `glDrawArrays` / `glDrawElements`.

---

## 3. The key mental model: who owns the EBO binding?

This answers *"can I use multiple arrays and tell it which one to follow, or do
all `glDrawElements` calls share the same array?"*

OpenGL is a state machine. Two binding rules that look similar but differ:

1. **`GL_ELEMENT_ARRAY_BUFFER` binding is stored *inside the VAO*.**
   Each VAO remembers its own EBO. When you bind VAO #2, the EBO binding
   switches to whatever was bound while VAO #2 was bound.
2. **`GL_ARRAY_BUFFER` binding is global and is *not* stored in the VAO.**
   But `glVertexAttribPointer` **captures** the currently-bound VBO into the
   attribute slot at the moment you call it. After that, un/rebinding
   `GL_ARRAY_BUFFER` does not affect the VAO.

Consequences:

- **Yes, you can have many (VAO, VBO, EBO) triples.** You "tell OpenGL which
  one to follow" with a single call: `glBindVertexArray(thatVao)`, then draw.
- All `glDrawElements` calls use **the EBO recorded in the currently bound
  VAO**. There is no per-draw EBO parameter in classic OpenGL (that only
  arrived with DSA in GL 4.5, e.g. `glVertexArrayElementBuffer`).
- Classic bug: calling `glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0)` *before*
  unbinding the VAO — this **erases** the EBO from that VAO. Unbind order
  should be: VAO first, then EBO/VBO.

Correct setup sequence (the one your library should encourage):

```cpp
glBindVertexArray(vao);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, ...);        // upload vertices
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, offset); // captures vbo
glEnableVertexAttribArray(0);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, ...);// upload indices (recorded in VAO)
```

---

## 4. Answers to the `P_glDrawElements` questions

### 4.1 What can `GLenum type` be?

It is the **element type of the indices stored in the EBO** — nothing else.
Exactly three legal values:

| `type` | C++ index type | Max index | Bytes/index |
|---|---|---|---|
| `GL_UNSIGNED_BYTE` | `uint8_t` | 255 | 1 |
| `GL_UNSIGNED_SHORT` | `uint16_t` | 65 535 | 2 |
| `GL_UNSIGNED_INT` | `uint32_t` | 4 294 967 295 | 4 |

It must **match the integer type you uploaded** into the EBO. Mismatch =
garbage geometry or crash. (This is why your square attempt with
`glDrawElements` was confusing: with no EBO bound the parameters are
meaningless anyway.)

**Can it be deduced automatically? Yes — if the EBO wrapper is templated** on
the index type (see Option C, §5.3). `Index_Buffer<uint32_t>` knows its
`type` is `GL_UNSIGNED_INT`; the programmer never writes it. A default
`GL_UNSIGNED_INT` in the struct is a reasonable fallback but invites silent
mismatch; deduction is strictly better.

### 4.2 Must `start_index` be `const void*`?

Historically yes, because `glDrawElements` predates buffer objects: when there
is *no* EBO, the parameter is a real CPU pointer to an index array (client-side
rendering, deprecated/invalid in core profile). **In core profile 4.3, an EBO
is always bound, and the parameter is reinterpreted as a byte offset into the
EBO.** So today it is never a real pointer — it's an integer wearing a pointer
costume.

Recommendation: **hide the `void*` inside the library**. Expose a strongly
typed offset and convert internally:

```cpp
// user-facing
GLintptr first_index{0};                    // index of the first element
// internally:
glDrawElements(mode, count, type,
               reinterpret_cast<const void*>(first_index * index_size));
```

Even better: express it as an *element index* (not a byte offset), because the
library knows `index_size` from the index type (again Option C). Then there is
no `void*` in the public API at all.

---

## 5. Design options for the drawing API

Goal you stated: **it must be evident which array (VAO/EBO) a draw call uses.**

### Option A — Keep free functions, pass the VAO explicitly (minimal change)

```cpp
struct P_glDrawElements {
  GL_Draw  mode{};
  GLsizei  count{};
  GLenum   type{GL_UNSIGNED_INT};   // still manual
  GLintptr first_index{0};          // element offset, no void*
};

void glDrawElements_cpp(GLuint vao, P_glDrawElements P);  // binds vao, draws
```

- ✓ Smallest diff from today; teaches the raw GL concepts.
- ✗ Still possible to pass a VAO that has no EBO → runtime failure.
- ✗ `type` still memorized by the programmer.
- Evidentness: the VAO is a visible argument. Good but not enforced.

### Option B — RAII objects, draw is a method on the VAO (recommended baseline)

Extends your existing `Buffers.hpp` (`Vertex_Array`, `Buffer`) with an
`Index_Buffer`, and moves drawing **onto the VAO**:

```cpp
Vertex_Array square;          // owns vao id, RAII
Buffer       vbo;             // positions
Index_Buffer ebo;             // indices (records its GL_ type)

square.bind();
vbo.upload(vertices);
square.attribute(0, 2, sizeof(Vertex), offsetof(Vertex, x));
ebo.upload(indices);          // bound while vao is bound → recorded in vao

square.draw_elements({ .mode = GL_Draw::LINE_LOOP, .count = 4 });
square.draw_arrays  ({ .mode = GL_Draw::TRIANGLES, .count = 6 });
```

`Vertex_Array::draw_elements(...)` does:
`glBindVertexArray(id); glDrawElements(...);`

- ✓ **Which array is used is syntactically obvious**: `square.draw_elements(...)`.
- ✓ Impossible to "draw with whatever VAO happens to be bound".
- ✓ RAII deletion matches the style `Buffers.hpp` already started.
- ✗ Still possible to call `draw_elements` on a VAO that never got an EBO —
  mitigate with a `bool has_index_buffer` flag → `throw std::logic_error`
  in debug.

### Option C — Typed `Index_Buffer<T>`: full deduction, no `void*`, no `GLenum type` (recommended end-state)

Make the EBO a template; the VAO keeps a pointer/handle to the bound
`Index_Buffer` and deduces everything:

```cpp
template<typename IndexT>   // uint8_t | uint16_t | uint32_t
class Index_Buffer {
  GLuint id{};
public:
  static constexpr GLenum gl_type = []{
    if constexpr (std::is_same_v<IndexT, uint8_t>)  return GL_UNSIGNED_BYTE;
    if constexpr (std::is_same_v<IndexT, uint16_t>) return GL_UNSIGNED_SHORT;
    else                                          return GL_UNSIGNED_INT;
  }();

  void upload(std::span<const IndexT> indices, GLenum usage = GL_STATIC_DRAW);
};

class Vertex_Array {
  GLenum index_type_{};      // 0 = no index buffer attached
  GLsizei index_count_{};
public:
  template<typename IndexT>
  void attach_indices(Index_Buffer<IndexT>& ebo, GLsizei count) {
    bind(); ebo.bind();                       // recorded in this VAO
    index_type_ = Index_Buffer<IndexT>::gl_type;
    index_count_ = count;
  }

  void draw(GL_Draw mode) const {                       // whole buffer
    glBindVertexArray(id);
    if (index_type_) glDrawElements((GLenum)mode, index_count_, index_type_,
                                    nullptr);           // offset 0
    else             throw std::logic_error("no index buffer attached");
  }
  void draw(GL_Draw mode, GLsizei count, GLintptr first_index = 0) const { /* partial */ }
};
```

Usage:

```cpp
Vertex_Array square;
Buffer vbo;  vbo.upload(vertices);
Index_Buffer<uint32_t> ebo;                 // type chosen once, here
square.attribute(0, 2, sizeof(Vertex), offsetof(Vertex, x));
square.attach_indices(ebo, indices.size());

square.draw(GL_Draw::LINE_LOOP);            // nothing else to remember
```

- ✓ `type` **deduced** from `IndexT` — the "memorize the GLenum" problem is gone.
- ✓ No `void*` anywhere in the public API.
- ✓ Wrong usage (draw elements with no EBO) is a **compile- or throw-time**
  error, not a GPU crash.
- ✗ Slightly more library code; templates mean header-only (fine for this lib).
- C++ note: `std::span` needs C++20 — you already compile with C++26.

### Option D — DSA style (GL 4.5+): `glCreate*`, `glNamedBufferData`, `glVertexArrayElementBuffer`

Bindless-of-bind: no `glBind*` at all; every call names the object.

- ✓ Cleanest modern API; errors impossible to order wrong.
- ✗ Requires GL 4.5 (you request 4.3 — intentionally, to match the book).
- ✗ Further from the book's teaching sequence.
- Verdict: revisit later; don't adopt now.

### Recommendation

- **Short term:** adopt **B** (methods on `Vertex_Array` + `Index_Buffer`),
  because it directly fixes the "which array is being used?" visibility problem
  with minimal new machinery.
- **When comfortable:** grow B into **C** by templating `Index_Buffer`, which
  removes `GLenum type` and `const void*` from the public surface entirely —
  your two stated pain points.
- Keep the free functions `glDrawArrays_cpp` / `glDrawElements_cpp` as a thin
  "teaching layer" (they mirror the book), but fix their parameter structs:
  - `P_glDrawElements::type` → document the 3 legal values (§4.1);
  - `start_index` → rename to `first_index` of type `GLintptr` (element
    offset), convert to `void*` inside the function.
- Add `PATCHES = GL_PATCHES` to `GL_Draw` (§1.1).

---

## 6. Side notes on `Project_07` itself (non-blocking)

- `main.cpp` line 46/48 reference `GL_Draw::GL_TRIANGLE_STRIP` / `GL_Draw::GL_LINES`
  in comments — those enumerator names don't exist (no `GL_` prefix inside the
  enum); would not compile if uncommented.
- `case 4` in your vertex shader is dead code for `LINE_LOOP` with `count = 4`
  (loop closes automatically); it would only matter for `LINE_STRIP` with 5
  vertices.
- Alternative square techniques worth a future project: `TRIANGLE_FAN` (4 verts,
  filled), `TRIANGLE_STRIP` (4 verts, filled), `TRIANGLES` + EBO (6 indices:
  0,1,2, 2,3,0 — the canonical indexed quad), or two triangles via geometry
  shader. The EBO version is the natural next exercise for this design.
