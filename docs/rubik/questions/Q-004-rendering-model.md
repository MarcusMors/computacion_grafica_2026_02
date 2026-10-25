# Q-004: How is the cube drawn?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

## Context
`my_lib` already has `Box` (one flat color per face), `Composite`, `Transform` and a `Renderer` that draws an outline. The
cube has 27 cubies, each with up to 3 colored faces and black inside.

## Options
### A. 27 `Box` figures, children of a `Rubik_Cube : Composite`
Faces that carry a sticker get the sticker color, the rest black; the outline can be black.
- (+) Uses the library as is; each cubie is one figure with a `Transform` taken from the state (Q-002 C).
- (-) 27 draw calls x 2 (fill + outline): fine here, but not a "pretty" look (no gaps, no rounded stickers).

### B. Cubies as black boxes plus a slightly inset colored `Square` per sticker
- (+) The classic look.
- (-) 27 + 54 figures; more code in the view layer.

### C. One generated mesh for the whole cube, rebuilt when the state changes
- (+) One draw call.
- (-) Mesh building logic; harder for a student to change.

## Recommendation
A first. B later if the group wants a nicer look (it only touches `rubik_view`).

## Answers
- Marcus:

- CristianMellado:

- FabricioMessa:

## Decision
