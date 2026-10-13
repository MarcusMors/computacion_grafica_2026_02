# LabCompiler — report of changes (Claude)

Scope: `my_lib` additions for **Project_11_3D_figures**, the Project_10 fixes you asked for, and a working Project_11 skeleton. Everything below compiles with the portable MinGW 16.2 / `-std=c++26` and was exercised (see [What I verified](#what-i-verified)).

---

## 1. The three things you asked for in Project_10 / Camera

| Request | What I did | Where |
|---|---|---|
| Replace E/Q with Left Shift / Left Ctrl | Removed the `E` and `Q` lines; Shift = up, Ctrl = down. Doc comment updated. | `my_lib/Camera.hpp` |
| White (or colored) outline on the pyramid | Outline now works. Color is one constant: `OUTLINE_COLOR` at the top of `main.cpp`. | `Project_10_pyramid/main.cpp`, `fragment_border.glsl` |
| Remove dynamic color | Removed `u_pulse` and the sine pulse. Each face is one plain color, no gradient, no lighting. | `fragment.glsl`, `main.cpp` |

### Why your outline attempt didn't draw (so you can explain it in class)
1. `program_white_ptr->use()` only *selects* a program. Nothing was ever **drawn** with it, and it never received `u_mvp`, so even a draw call would have collapsed to a point.
2. `fragment_border.glsl` had the color hard-coded to **black** (the comment said "white").
3. Even once drawn, a line lying exactly on a face has the *same depth* as the face → **z-fighting** (flicker / missing segments). Fix: `glPolygonOffset` pushes only the *filled faces* slightly back.
4. Outline needs **line primitives**. I put 18 triangle indices followed by 16 edge indices in the **same** index buffer and draw them with `draw_elements(TRIANGLES, 18, 0)` and `draw_elements(LINES, 16, 18)` (your `Vertex_Array` already supported ranges).

### Flat color per face ⇒ more vertices
A vertex holds one color, so a corner shared by faces of different colors must be duplicated per face: the pyramid went from 5 to **16 vertices** (4 base + 4 × 3 sides). Same reason a box has 24, not 8. This is the standard way to get flat color without lighting code.

### Other fix in Project_10
`key_callback` printed `POINT.x / POINT.y`, a leftover from an older project. `POINT` doesn't exist there, so **the file did not compile as I received it**. It now just prints the key name.

---

## 2. New library files (all in `my_lib/`, header-only like `Camera.hpp`)

| File | Purpose |
|---|---|
| `Mat4.hpp` (extended) | `radians()/degrees()`, `enum class Axis {X,Y,Z,NEG_X,NEG_Y,NEG_Z}`, `mat4_rotate(Axis, rad)` / `mat4_rotate_x/y/z`, `mat4_scale(Vec3)` / `mat4_scale(float)`, `Mat4 * Vec4`, `transform_point()` (w=1), `transform_direction()` (w=0), chainable `Mat4::identity().translate(..).rotate(Axis::Y, ..).scale(..)` (glm-style: each call applies *after* the previous). More `Vec3` operators (`-v`, `+=`, `/`, component-wise `*`). Existing functions untouched. |
| `Transform.hpp` | `Transform{ position, rotation_deg, scale }` → `matrix()` = T·R·S. `rotate(Axis, deg)`, `translate()`, `frame()`. Also `Frame {right, up, forward}`: the orthonormal basis. |
| `Animation.hpp` | `pulse(time, freq, amplitude, center)` (sinusoid) and `spin(time, deg_per_sec)`. Stateless: assign the result to a transform every frame. |
| `Mesh.hpp` | CPU geometry: `Mesh` (vertices, triangles, edges), `add_polygon()`, `centroid()`, `bounds()`, `Aabb`, and a `color::` namespace (`color::red`, …). No OpenGL needed. |
| `Shapes.hpp` | `make_box`, `make_pyramid`, `make_sphere` (songho UV sphere), `make_triangle`, `make_square`, `make_pizza`. |
| `Renderer.hpp` | `Gpu_Mesh` (VAO+VBO+EBO) and `Renderer` (built-in shaders, faces + outline, `begin_frame(window, camera)` does viewport/clear/aspect/camera in one call). |
| `Figure.hpp` | `Figure` interface, `Mesh_Figure`, `Box`, `Pyramid`, `Sphere`, `Triangle`, `Square`, `Pizza`, `Composite`. |
| `Shader_Source.hpp` (1 small change) | Added a `Shader_Source(std::string_view)` constructor so the renderer can embed GLSL as C++ raw strings. `#embed` usage is unaffected. |

New project: `glfw-master/OwnProjects/Project_11_3D_figures/` (`main.cpp`, `Castle.hpp`, `CMakeLists.txt`), registered in `OwnProjects/CMakeLists.txt`.

### How to use it (the whole frame)
```cpp
Renderer renderer;  Castle castle;  Sphere ball{ 1.0f, color::red, color::yellow };   // in init()

ball.transform.scale = Vec3{1,1,1} * pulse(time, 0.5f, 0.15f);       // animate
castle.transform.rotation_deg.y = spin(time, 10.0f);

renderer.begin_frame(window, camera);                                  // every frame
castle.draw(renderer);  ball.draw(renderer);
```
Project_11 keys: **WASD** move, **Shift/Ctrl** up/down, **arrows** look, **O** outline on/off, **T** add a tower, **R** remove last tower, **C** print castle centroid + bounding box.

---

## 3. Composite objects — the answer to "how do we add/remove figures?"

`Castle : Composite`, and `Composite::add<T>(T figure, position)` / `remove(part)`:

```cpp
class Tower : public Composite {
public:
  Tower() {
    add(Box{ {1, 3, 1}, color::stone },      { 0, 1.5f, 0 });
    add(Pyramid{ 1.4f, 1.2f, 1.4f, color::red }, { 0, 3.6f, 0 });
  }
};
Tower &t = castle.add(Tower{}, { 3, 0, 3 });   // returns the exact type
castle.remove(t);
```
* `add<T>` is a **constrained template** (`std::derived_from<Figure>`): adding a non-figure is a *compile-time* error, and you get back `T&` (not a base pointer).
* Parts are stored behind the common `Figure` interface (`unique_ptr<Figure>`), so one list mixes boxes, spheres and whole sub-composites, and add/remove work at run time. Since you said you're happy to edit source rather than change things at run time, you can also just build everything in the constructor, as `Castle.hpp` does.
* Nesting works (`Castle` contains `Tower`s which contain a `Box` and a `Pyramid`); transforms multiply down the tree.
* **Orthonormal vectors:** every `Figure` has `frame()`. A `Composite` uses its own transform by default, or `use_frame_of(other_figure)` to borrow another figure's axes (`castle.use_frame_of(castle.keep())`). `move_along_frame({0,0,1})` moves along those axes.

---

## 4. ⚠️ Decisions I need from you

1. **Definition of "centroid".** I implemented it as the **mean of the figure's distinct corner points** (duplicated per-face vertices are counted once). For a box/sphere this is the geometric center; for a **pyramid it is *not* the center of mass** (for a unit pyramid spanning y = −0.5…0.5, the vertex mean is at y = −0.3, the center of mass at y = −0.25). For a `Composite` it's the mean of its parts' centroids (each part weighs the same). If your course defines centroid as center of mass / volume, tell me and I'll switch `Mesh::centroid()`.
2. **Bounding box = axis-aligned (AABB).** `bounds()` is exact in the figure's own space. After a *rotation* the world-space box (`bounds_in_parent()`) is the AABB of the 8 rotated corners, which is **not the minimal box** any more. If "minimal bounding box" means oriented (OBB) or minimal-volume, that's a bigger feature (PCA / rotating calipers) — say if you want it.
3. **Gimbal lock.** You asked for a plain `if`. Done in two places with one shared constant `MAX_PITCH_DEG = 89` (`Mat4.hpp`): `Camera::process_keyboard` (already there, now uses the constant) and `Transform::rotation_matrix()`. **Consequence:** a figure's *X* rotation (pitch) is limited to ±89°, so a ball can't roll a full turn about X through `Transform`. Y and Z turn freely. Options: (a) keep as is, (b) change the Euler order so the clamped axis is the one you rotate least, (c) store the rotation as a matrix (no gimbal lock at all, no `if`). I chose the `if` because you asked for it.
4. **Composite storage: virtual interface vs. templates.** I went with a virtual `Figure` (runtime add/remove) + a *templated* `add<T>` (compile-time check). A fully compile-time `Composite<Parts...>` (tuple) is possible but can't loop ("4 towers") or remove parts. Tell me if your teacher expects pure templates.
5. **Shapes are centered on their own origin** (a unit box spans −0.5…0.5), not sitting on y = 0. That makes rotation/centroid natural but means you offset by half the height when stacking (see `Tower`). Alternative: origin at the base.
6. **Sphere coloring.** Each patch is a flat color; I alternate two colors like a checkerboard so rotation is visible. `Sphere(r, one_color)` gives a solid one. 16×8 in the demo; the default is 24×12. The pole patches are triangles; the outline becomes dense near the poles — turn it off per figure with `sphere.outline = false`.
7. **Outline policy.** Renderer-wide (`renderer.outline`, `outline_color`, `line_width`) with a per-figure override (`fig.outline`, `fig.outline_color`). Is a global toggle enough for the class?

---

## 5. Things I'd like you to review

* **GL objects need a context.** Figures own VAOs/VBOs, so create them in `init()`. In Project_11 everything lives in one `Scene` held by a `unique_ptr`, which is `reset()` **before** `glfwDestroyWindow` (otherwise the destructors call `glDelete*` on a dead context).
* **`Program` is copyable by accident** (it has a destructor that deletes the GL program, but no deleted copy). Copying one would double-delete. I deleted copying in `Renderer`, but didn't change `Program`. Suggest `Program(const Program&) = delete;` in `_cpp.hpp`.
* **`glLineWidth(2)`**: core-profile drivers are allowed to clamp it to 1. I did not measure the line thickness on your GPU; if the outline looks thin, that is why.
* Project_10's camera starts at y = 0.5, z = 3, so the pyramid's base is slightly cut off by the window bottom. That was already the case; I left it.
* Pre-existing oddities left alone: `main()` calls `glfwPollEvents()` twice per frame; `exit()` is called before the (disabled) `log_file.close()`; `using namespace std;` inside `_cpp.hpp`/`Shader.hpp`; `-Wunused-parameter` warning in `Shader.hpp:56`.
* **I overwrote** `glfw-master/bin/Project_10_pyramid.exe` and created `Project_11_3D_figures.exe` (built from a scratch build dir, not your `build/` folder). Your usual `_GENERATE_MINGW_x64.bat` will rebuild both. Nothing was committed.

## 6. What I verified

* **CPU unit checks** (compiled and run): box/pyramid/sphere/triangle/square/pizza vertex, triangle and unique-edge counts; every face winds counter-clockwise from outside; centroids and bounds; all six `Axis` rotations (including the NEG ones), point vs. direction transforms, chained matrix order, pitch clamp, frame orthonormality, `Aabb::transformed`, `pulse()`. All passed.
* **Real OpenGL 4.3 offscreen renders** of Project_10 and Project_11 (hidden window + `glReadPixels`): faces are plain colors, outline visible in white, hidden edges stay hidden, castle/sphere/pizza/square/triangle all drawn. Tower add/remove (`T`, `R`) and centroid/bounds (`C`) were exercised through `key_callback`: 4 + 3 − 1 = 6 towers, values sensible.
* **Not verified:** interactive keyboard feel (WASD/Shift/Ctrl) on a live window. The change is two lines mirroring existing ones. My screen-capture couldn't see GL windows, which is why I used offscreen renders.

---

## 7. Update — Project_12 (jumping towers), color palette

* **`my_lib/Colors.hpp` (new):** `colors::white`, `colors::cyan`, `colors::pink`, `colors::dark_brown`, `colors::stone`, ... (about 45 CSS-style names) and `colors::rgb(255, 105, 180)` for any other color. **Breaking rename:** the old `color::` namespace is now `colors::` (library and Projects 11/12 updated; Project_10 never used it). Colors are now the standard CSS values, so e.g. `red` is pure (255,0,0) and looks stronger than before; tower roofs use `crimson`.
* **`Animation.hpp`:** new `hop(time, period, height, delay, airtime)`, a parabolic vertical jump that is always >= 0.
* **`Castle::towers()`:** returns every tower so each can be animated separately (Project_11 and 12 copies).
* **Project_12:** the 4 corner towers (and any added with `T`) hop vertically, each starting 0.3 s after the previous one. Ground is a thin `Box` because a `Square` laid flat needs a -90 degree X rotation, which the +/-89 degree pitch guard (decision 3 above) would clamp to -89.

* **Project_13 / `heartbeat()`:** `Animation.hpp` gains `heartbeat(time, bpm, amplitude, base)`, a "lub-dub" scale factor built from two sin^2 swells plus a rest. Project_13 adds an orbiting heartbeat ball, a flat spinning "sun" (Pizza, no outline) at the far right and a triangle turning about Y above the castle, on top of Project_12's jumping towers.

---

## 8. Update — pedagogic_visualizations_look_at (cube seen from a camera, boost fractions)

* **New project:** `glfw-master/OwnProjects/pedagogic_visualizations_look_at/` (registered in `OwnProjects/CMakeLists.txt`). Camera position, target, up vector, cube half-size, tick step and tick range are variables at the top of `main.cpp`, stored as **integers in thousandths** and wrapped in `boost::rational<int>` (so `500` prints as `1/2`).
* **Boost:** only the 53 headers `<boost/rational.hpp>` needs (Boost 1.92.0) were copied to `glfw-master/deps/boost/` (518 KB, license included). It is header-only. Note `.gitignore` ignores `deps/*`, so Boost is **not** in git; it travels with the folder. Remove the exception if you want it tracked.
* **New library pieces:** `my_lib/Draw2D.hpp` (pixel-space lines, dashed lines, translucent polygons, 5x7 bitmap text) and `mat4_ortho()` in `Mat4.hpp`.
* **Assumptions to confirm:**
  1. *Axes on the borders:* each world axis gets a ruler on ONE border of the square frame; a tick for the value t sits where the world point t*axis lands on screen (only its screen-x counts on the bottom/top borders, its screen-y on the left/right ones). Mostly-horizontal axes take bottom (then top), mostly-vertical ones left (then right). Press **G** to see the real axes and drop lines. Ticks therefore bunch up with distance in perspective.
  2. Perspective is the default, orthographic is on **P** (same size as perspective at the target). Vertical FOV 45 deg, near 0.1, far 100.
  3. Hidden edges are decided exactly for a convex solid: an edge is hidden when both adjacent faces point away from the camera.
  4. Axis colors: x red, y pure green (`lime`), z `royal_blue` (pure `blue` is unreadable on black). Swap in the constants if you prefer.
* **Debug builds:** a fully static `-O0` link with `boost::rational` fails with "multiple definition of std::logic_error::logic_error"; the project's CMakeLists adds `-Wl,--allow-multiple-definition` for it (Release builds never needed it).

---

## 9. Update — full Boost, exact 2D coordinates, the "exam" setup

* **Boost (all headers) is now part of LabCompiler:** `LabCompiler/Boost/boost_1_92_0/boost/...` (16 415 files, 158 MB, like `MinGW/` and `Cmake/`). `my_lib/CMakeLists.txt` adds it to the include path of every project linking `my_lib` (highest `boost_*` version wins). `/Boost/` is in `.gitignore` and skipped by `_SNAPSHOT.ps1`. The 53-header subset in `glfw-master/deps/boost` was removed. Compiled Boost libraries (Filesystem, Thread, Program_options, ...) are NOT built.
* **pedagogic_visualizations_look_at:** `SETUPS` table (key **N**): setup 0 = camera (2,3,5); setup 1 = the classic exercise (pinhole at the origin, window [0,1]x[0,1], off-axis frustum), which reproduces A(1/4,1/2) ... H(1/2,2/3) exactly. New `mat4_frustum()` in `Mat4.hpp`.
* **SCREEN rulers (key M, default):** the 2D coordinates 0..1 on the borders with a grid; tick step 1/N where N is the least common denominator of all coordinates (12 here). **D** shows labels over N (2/12) instead of reduced (1/6). **L** prints each vertex's coordinates next to it.
* **Exact arithmetic:** when the camera basis and the window are rational the whole projection is computed with `boost::rational<long long>`; otherwise the console prints decimals (the look-at basis then contains square roots).

---

## 10. Update — split window with a free camera

* **pedagogic_visualizations_look_at** now opens a 1600x800 window with two square panes: LEFT = the camera's view (as before), RIGHT = a free camera (`Camera` from my_lib: W A S D, Left Shift / Left Ctrl, arrows) with the x, y, z axes always drawn (ticks and labels at every whole number from -5 to 5), the cube, and the left camera drawn as a point `V`, a cyan pyramid of what it sees and a dashed line to its target.
* **Keys added:** `V` free camera jumps to V looking at the target (fov chosen to fit the left window), `B` back to the bird's-eye start, `O` rays from V to every vertex, `,` / `.` free-camera field of view. **`D` (label denominators) moved to `C`** because D is also "strafe right" for the free camera.
* **Both panes use the same drawing functions** (`draw_cube`, `Pane`); the near plane is clipped, so edges and faces do not vanish when you fly close.
* **Limits:** the free camera cannot roll (it always keeps +Y up) and is always perspective; for the off-axis EXAM setup the symmetric free view shows the left window as a crop; with the camera at the origin the axes pass through it and are not visible.

---

## 11. Update � three setups on keys 8 / 9 / 0, cube axes vs camera axes

* **Keys:** `N` is gone. **8** = setup 0 (camera c1 at (2,3,5), the default), **9** = the professor's / exam setup, **0** = *c1 at the origin*: setup 0 with the WHOLE world moved by its look-at matrix (camera at the origin looking down -Z, the cube rotated, the original world axes carried along). Its left picture is identical to setup 0's (P*V*p = P*I*(V*p)); the console "camera space" column of setup 0 equals the scene coordinates of setup 2.
* **Key X (right pane):** axes shown = the CUBE's axes (solid X, Y, Z) -> the CAMERA's axes (dashed XC, YC, ZC: right, up, backwards from the camera) -> both. In setup 9 they coincide; in setup 0 they are the same rotated frame; in setup 8 the camera axes sit at V.
* `Cube` now keeps its exact original coordinates (`vertex`) and the positions in the scene (`pos`, with `transform()` for the rigid move); the table's "world (exact)" column always shows the original exact coordinates of C.

---

## 12. Fixes � orthographic on views 9 / 0, meaning of "cube axes"

* **Orthographic was wrong in the exam setups (9) and in c1-at-the-origin (0).** The orthographic window was scaled by the distance to the *target*; in those setups the target is just a point one unit in front of the camera, so the cube was cropped or filled the whole pane. It is now scaled by the depth of the cube's centre (`App::ortho_distance()`), the distance at which it matches the perspective picture. The exact-fraction path uses the same depth as a fraction: in setup 9 the orthographic 2D coordinates are 1/5, 2/5, 3/5, 4/5 (far and near faces coincide).
* **Key X, "cube axes":** they are now the frame attached to the cube (through its centre, along its edges), labelled with WORLD coordinates. Before, they were the world axes, which in setup 9 coincide exactly with the camera's axes, so cycling showed no difference. Setups 8 and 0: unchanged (the cube is centred on the world origin). Setup 9: the cube's axes are parallel to the camera's but offset to the cube's centre.
