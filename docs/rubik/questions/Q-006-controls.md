# Q-006: How does the user control the cube and the camera?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

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

- CristianMellado: Elijo la Opción A: solo teclado. No requiere trabajo adicional en la librería y nos permite concentrarnos en el cubo. Los controles serían: U D L R F B para movimientos, Shift + letra para el inverso, 2 + letra para el half turn, flechas para orbitar la cámara, Space para scramble, Backspace para undo, Enter para reset, y H para mostrar ayuda en pantalla. El mouse dragging requeriría ray casting y picking que es demasiado trabajo extra fuera del scope del curso.
- FabricioMessa:

## Decision
