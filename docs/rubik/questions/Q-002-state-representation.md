# Q-002: How do we store the state of the cube?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

## Context
The central data structure: everything is built on it (moves, drawing, animation, undo). It should be easy to read, to print and
to test without OpenGL. Depends on Q-001 (3x3x3) and Q-008 (scope).

`to_facelets()` (the 54 stickers, face by face) is also what the solver is given (Q-011). With the middle layer turning (Q-010)
the centers move, so "solved" means every face is one color, not "the standard color scheme".

The state layer needs no `my_lib` header: `my_lib`'s `Vec3` lives in `Buffers.hpp`, which includes glad. Small integer types of
our own (a position, a rotation) keep `rubik_state` compilable and testable on a machine without a GPU.

## Options
### A. Facelet array: `std::array<Color, 54>`
Each move is a fixed list of index cycles (a table per move).
- (+) Trivial to print and debug; easy to explain.
- (-) Pieces have no identity: animating "this cubie" needs extra bookkeeping; the tables are easy to get wrong.

### B. Cubie model used by solvers: 8 corners (position + twist 0..2), 12 edges (position + flip 0..1)
- (+) Compact; legality checks; the base of real solvers.
- (-) Hardest to explain; colors come from lookup tables; centers are not represented.

### C. 27 cubies, each with an integer position `(x, y, z)` in {-1, 0, 1} and an integer rotation matrix
```cpp
struct Cubie { Int3 position; Rotation rotation; };   // position: x, y, z each -1, 0 or +1
std::array<Cubie, 27> cubies;                         // rotation: starts as "not turned"

// a move: every cubie whose coordinate on the face's axis is on that layer turns 90 degrees about the axis
for (Cubie &c : cubies)
  if (is_in_layer(c.position, move)) { c.position = turn(c.position, move); c.rotation = turn(move) * c.rotation; }
```
A move = "rotate every cubie whose coordinate on that axis equals the layer by 90 degrees about the axis". A quarter turn maps whole
numbers to whole numbers, so everything stays an integer: no float error accumulates (Q-005) and two states compare exactly.
- (+) The same data drives logic and drawing; moves are geometry (the topic of the course), with no tables; animation falls out naturally.
- (-) Stickers are derived (position + rotation -> which color faces where); a solver would convert to B.

## Recommendation
C for the state, plus a function `to_facelets()` (54 colors) used for printing, tests and as the input of the solver (Q-011). How the view turns a cubie's rotation
into a drawing is Q-009.

## Answers
- Marcus:

- CristianMellado:

- FabricioMessa:

## Decision
