# Q-006: How does the user control the cube and the camera?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, F_Messa

## Context
`my_lib` supports keyboard input only (a polled `Camera` for W A S D / Shift / Ctrl / arrows, and one-shot key callbacks).
It has no mouse picking. Letters U, D, L, R, F, B are the natural move keys, but **D conflicts with strafing** in `Camera`.

## Options
### A. Keyboard only
Move keys `U D L R F B` (hold Shift for the counter-clockwise move, key `2` before the letter for a half turn); arrows orbit the
camera; Space scrambles; Backspace undoes; Enter resets. A help text on screen lists the keys.
- (+) No library work; moves are one-shot events (callback).
- (-) Less "natural" than dragging.

### B. Add mouse picking and dragging
- (+) The way people expect to play.
- (-) Ray casting, cubie picking and drag direction: a lot of new library code.

## Recommendation
A, with the camera orbiting around the cube on the arrow keys (so no clash with the move keys).

## Answers
- Marcus:
- CristianMellado:
- F_Messa:

## Decision
