# Q-003: How is a move represented, and can we type `R U R' U2`?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa, after Q-010

## Context
Moves are used by input (keys), scramble, undo, animation, tests and the solver's answer. The standard notation has 6 faces x 3
amounts = 18 moves; the course also wants the three middle slices M, E, S (Q-010), which makes 9 "faces" x 3 amounts = 27 moves.

**Direction.** `R` is clockwise *seen from outside the R face*. `my_lib` rotations follow the right-hand rule (`Mat4.hpp`):
a positive angle is counter-clockwise seen from the tip of the axis. So a face turn is a NEGATIVE angle about that face's outward
axis, and the six values of `Axis` are exactly the six outward directions: R = `X`, L = `NEG_X`, U = `Y`, D = `NEG_Y`,
F = `Z`, B = `NEG_Z`. A middle slice uses the axis of the face it follows (M like L, E like D, S like F), on the layer in the
middle. The sign is the easiest thing to get backwards; a test "R then R' gives back the start" does not catch it,
but "after R the cubie that was at the front-top-right corner is at the back-top-right" does.

## Options
### A. An enum with 18 values (`R`, `R_prime`, `R2`, ...)
- (+) Easy to read in a `switch`.
- (-) 18 cases everywhere; the inverse and the angle need tables.

### B. A small struct `Move { Face face; int quarter_turns; }` with `quarter_turns` in {1, 2, 3}
```cpp
Move m{ Face::R, 3 };            // R'  (three quarter turns clockwise = one counter-clockwise)
inverse(m)  -> { Face::R, 1 }    // 4 - quarter_turns
angle_deg(m) -> -270.0f          // -90 * quarter_turns, about the outward axis of the face
parse("R U R' U2 M")  /  to_string(m)    // Face is U D L R F B M E S; parse() also reads the solver's answer (Q-011)
```
- (+) Three lines of logic; tests can be written as text.
- (-) A value like 0 or 5 must be refused.

## Recommendation
B.

## Answers
- Marcus:

- CristianMellado:

- FabricioMessa:

## Decision
