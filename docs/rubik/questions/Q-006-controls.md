# Q-006: How does the user control the cube and the camera?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa, after Q-003, Q-008, Q-010 and Q-011

## Context
The existing projects read the keyboard two ways (see Project_13): a callback registered with `glfwSetKeyCallback` for
one-shot events, and polling with `glfwGetKey` for held keys (`Camera::process_keyboard`: W A S D, Shift, Ctrl, arrows).
For the mouse, `Window` offers `cursor_position()` and `is_mouse_button_pressed()`, but nothing for picking.

`Camera` is a fly-through camera (position + yaw/pitch); an orbit around the cube does not exist yet. Letters U, D, L, R, F, B
are the natural move keys. They clash with `Camera::process_keyboard` (D strafes, Shift goes up) only if that function is called;
the options below do not call it.

## Options
### A. Keyboard only
Move keys `U D L R F B` and the slices `M E S` (hold Shift for the counter-clockwise move, tap `2` before the letter for a half
turn); arrows orbit the camera; Space scrambles; Enter solves (plays the solution of Q-011); Backspace undoes and Delete resets
(if Q-008 says so). A help text on screen lists the keys (`Draw2D` has a text function).
The orbit is a small function inside the project, about 10 lines: it sets `camera.position`, `yaw_deg` and `pitch_deg` from an
azimuth, an elevation and a distance, always looking at the cube. No `my_lib` change.
- (+) No library work; moves are one-shot events (callback).
- (-) Less "natural" than dragging.

### B. A, and dragging with the mouse button down also orbits the camera
Uses the polling functions `Window` already has; no picking.
- (+) Natural way to look around; few lines more.
- (-) Two ways to do the same thing (arrows and mouse).

### C. Mouse picking and dragging the layers
- (+) The way people expect to play.
- (-) Ray casting, cubie picking and drag direction: a lot of new library code.

## Recommendation
A, with the camera orbiting around the cube on the arrow keys (so no clash with the move keys). B is a cheap addition later,
inside `main.cpp` only.

## Answers
- Marcus: A, solo teclado. Sobre qué teclas aún está abierto eso. Hice una propuesta en una pregunta anterior.

- CristianMellado:

- FabricioMessa:

## Decision
