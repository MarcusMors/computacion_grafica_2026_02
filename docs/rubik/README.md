# Topic: Rubik's cube (group project)

**Goal.** A Rubik's cube in OpenGL, written by three students on top of `my_lib`, structured so that every part is easy to
use, understand and modify, the way the library itself is. The project lives in `glfw-master/OwnProjects/rubik_cube/` (not
created yet).

- **Team:** Marcus (GitHub `MarcusMors`), CristianMellado, FabricioMessa.
- **Facilitator:** Marcus's agent writes the questions; everybody answers (see [`../README.md`](../README.md)).
- **Library rule:** if the project needs something new in `my_lib`, that is a question first and a pull request second.
- **Tiebreaker:** when two options both work, we pick the one that is easier to understand, even if it is slower. (Marcus's
  rule; Cristian and Fabricio confirm or object in the pull request.)

## Course requirements (relayed by Marcus; Cristian and Fabricio confirm in the pull request)

1. The middle layer turns too (M, E, S), not only the six outer faces (Q-010).
2. For the presentation the program shows the cube going from scrambled to solved as an animation, every move displayed (Q-008, Q-011).
3. An outside solver may be used, but the teacher warned that probably no solver outputs middle-layer moves (Q-012).

## Where we are

No code yet. First step is the structured interview: answer the questions in [`questions/`](questions/). The order to answer
them in, and who leads which, is in [`START_HERE.md`](START_HERE.md).
Use the words in [`GLOSSARY.md`](GLOSSARY.md); add a term there (in a pull request) when you invent one.

**To answer the questions, read [`START_HERE.md`](START_HERE.md).**

## Proposed layers (to be confirmed by Q-007)

```
rubik_state      pure data and rules, no OpenGL: faces, moves, applying a move, "is it solved?", scramble
rubik_view       turns the state into figures (my_lib Composite / Box), colors and transforms
rubik_animation  one layer turn over time, a queue of turns, easing
rubik_solver     solve(state) -> moves: the one function between the solver (Q-011) and the rest
main.cpp         window, keyboard, camera, on-screen help
```
