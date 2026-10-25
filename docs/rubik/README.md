# Topic: Rubik's cube (group project)

**Goal.** A Rubik's cube in OpenGL, written by three students on top of `my_lib`, structured so that every part is easy to
use, understand and modify, the way the library itself is. The project lives in `glfw-master/OwnProjects/rubik_cube/` (not
created yet).

- **Team:** Marcus (GitHub `MarcusMors`), CristianMellado, F_Messa.
- **Facilitator:** Marcus's agent writes the questions; everybody answers (see [`../README.md`](../README.md)).
- **Library rule:** if the project needs something new in `my_lib`, that is a question first and a pull request second.

## Where we are

No code yet. First step is the structured interview: answer the questions in [`questions/`](questions/) (they are in
dependency order: scope, then state, then moves, then rendering, then animation, then controls, then who builds what).
Use the words in [`GLOSSARY.md`](GLOSSARY.md); add a term there (in a pull request) when you invent one.

## Proposed layers (to be confirmed by Q-007)

```
rubik_state      pure data and rules, no OpenGL: faces, moves, applying a move, "is it solved?", scramble
rubik_view       turns the state into figures (my_lib Composite / Box), colors and transforms
rubik_animation  one layer turn over time, a queue of turns, easing
main.cpp         window, keyboard, camera, on-screen help
```
