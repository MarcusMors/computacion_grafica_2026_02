# Q-004: How is the cube drawn?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

## Context
`my_lib` already has `Box`, `Composite`, `Transform` and a `Renderer` that draws an outline per figure (its color can be set per
figure). A `Box` takes six face colors in the order +Z, -Z, +X, -X, +Y, -Y, which is exactly F, B, R, L, U, D in our world axes
(see the glossary). The cube has 27 cubies, each with up to 3 colored faces and black inside. Depends on Q-002 (C).

Two limits of the library shape the options, and Q-009 decides what to do about them: `Transform` cannot hold every rotation
a cubie reaches, and `Composite` draws all its parts alike.

**Vocabulary** (for readers who know data structures but not graphics yet):
- A **mesh** is an indexed face list: a vector of vertices (here a 3D position and a color) plus a vector of triangles, each one
  3 indices into the vertex vector. `Mesh` (Mesh.hpp) is exactly that, on the CPU, with no OpenGL.
- `Box` builds a mesh of 6 quads. Each quad gets its own 4 vertex copies, because a vertex holds ONE color: 24 vertices, not 8,
  and every face stays one flat color.
- `Gpu_Mesh` copies the mesh once into graphics memory; drawing it afterwards takes a couple of calls. A **figure** is a shared
  `Gpu_Mesh` plus a `Transform` (where it is, how it is turned).
- To draw, the program multiplies the figure's 4x4 **model matrix** by the camera's matrices, and the GPU applies the product to
  every vertex. Moving a cubie never touches its mesh; only its matrix changes.

## Options
### A. 27 `Box` figures, one per cubie
Faces that carry a sticker get the sticker color, the rest black; the outline can be black. A cubie never changes its colors,
only its pose (Q-002 C), so each `Box` is built once at start. Who owns the 27 boxes and how they are posed: Q-009.
- (+) Uses the library as is; each cubie is one figure.
- (-) 27 figures drawn as fill + outline (54 draw calls): fine here, but not a "pretty" look (no gaps, no rounded stickers).

### B. Cubies as black boxes plus a slightly inset colored `Square` per sticker
- (+) The classic look.
- (-) 27 + 54 figures; more code in the view layer. A `Square` lies in the XY plane, so the stickers on the top and bottom
  faces need a rotation of +/-90 degrees about X, which `Transform` clamps to 89 (Project_13 works around the same limit
  with a `Box` for its ground). Only works with a Q-009 answer that does not use Euler angles.

### C. One generated mesh for the whole cube, rebuilt when the state changes
- (+) One draw call.
- (-) Mesh building logic; harder for a student to change.

## Recommendation
A first. B later if the group wants a nicer look (it only touches `rubik_view`).

## Answers
- Marcus: I would say option A. Because we can keep improving the library and as my partners said, we might start with it and then move or adapt as we need. The thing is to allow the student see the library and use it and understand it.

- CristianMellado:

- FabricioMessa:

## Decision
