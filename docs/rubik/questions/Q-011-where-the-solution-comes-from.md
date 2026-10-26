# Q-011: Where does the solution come from?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa, after Q-002, Q-003 and Q-010

## Context
The course asks (relayed by Marcus) that the program shows the cube going from scrambled to solved, every move displayed
(Q-008). The teacher said we may use an outside solver. Whatever we pick, the rest of the program needs only ONE function, the
**seam** between the solver and us:

```cpp
std::vector<Move> solve(const Cube_State &cube);   // the moves that take `cube` to a solved state
```

The animation (Q-005) plays what it returns, one move at a time.

Facts that shape the options:
- **Solvers assume the standard centers.** They expect the center of the U face to carry the "U" color. After a middle-layer
  turn that is false (M carries the back center to the top). So `solve()` first renames each sticker's color to the face whose
  center has that color, then asks the solver. After solving, every face is one color (glossary), possibly not the standard scheme.
- **Third-party code stays out of git** (porting ADR-0005): it would come with the LabCompiler zip, which must then be re-issued, or
  a new ADR must make an exception.
- **Solvers output face moves only**, no middle-layer moves. What to do about that is Q-012.

## Options
### A. Replay the history backwards ("unscramble")
The program already records every move since the cube was last solved (scramble and player moves; Q-008 B needs the same list).
`solve()` returns that list reversed, each move inverted: `R U M'` becomes `M U' R'`. Moves that cancel (`R` then `R'`) are removed first.
- (+) No outside code; a few lines; works with the middle layer for free (a scramble containing `E` is undone by `E'`); every
  move is displayed; the demo cannot fail.
- (-) It is not a solver: the "solution" is as long as the history (a 25-move scramble gives 25 moves, a real solver finds about
  20 for any cube). Ask the teacher whether "unscramble" counts as solving.

### B. A solver library compiled into the project
The best-known method is Kociemba's two-phase algorithm: it searches with large precomputed tables and returns a solution of about
20 moves, not guaranteed to be the shortest. Candidate: the C code in the `ckociemba` folder of
[muodov/kociemba](https://github.com/muodov/kociemba).
- Checked on its GitHub page (2026-10-25): license **GPL-2.0**; pure C; input is a 54-letter string in the order U1..U9 R1..R9 F1..F9
  D1..D9 L1..L9 B1..B9 (solved = `UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB`); output is moves like `R U R' U R U2 R'`.
- NOT checked: whether the C code can be called as a function from our program (the page only describes a command-line program),
  how long the first run takes to build its tables, how much memory it uses.
- Other candidates I looked at: a C# solver (tremwil/TwoPhaseSolver, MIT) is the wrong language; I found no confirmed C or C++
  port of min2phase.
- (+) A real solver; ~20 moves for any state; our `to_facelets()` (Q-002) already produces its input and `parse()` (Q-003) reads its output.
- (-) GPL-2.0: a program that includes it is distributed under GPL-2.0 (usually fine for a course; check if the code is published).
  Third-party sources must reach everybody through the zip. Needs the three checks above first.

### C. A solver program that we start and whose output we read
The same C code built by its own `make` as a command-line program. Our program starts it, gives it the 54 letters, reads the moves.
- (+) Its code never mixes with ours.
- (-) Starting a program works differently on Windows, Linux and macOS (it touches the porting topic); the executable must travel
  with ours; two things to build.

### D. Write our own solver
For example the layer-by-layer method people learn first: white cross, white corners, middle edges, and so on.
- (+) Nothing third-party; every step can be explained in class.
- (-) A project in itself, and the solutions are long (roughly a hundred moves).

## Recommendation
A first, behind `solve()`: it works in a day and the demo (including the middle layer) cannot fail. Then B if the teacher expects a
real solver; switching is one function because the rest of the program only knows `solve()`. C and D are not recommended.

## Answers
- Marcus: Empecemos con A, and after having a good cube, doing B

- CristianMellado:

- FabricioMessa:

## Decision
