# Q-002: How do we store the state of the cube?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

## Context
The central data structure: everything is built on it (moves, drawing, animation, undo). It should be easy to read, to print and
to test without OpenGL. Depends on Q-001.

## Options
### A. Facelet array: `std::array<Color, 54>`
Each move is a fixed list of index cycles (a table per move).
- (+) Trivial to print and debug; easy to explain.
- (-) Pieces have no identity: animating "this cubie" needs extra bookkeeping; the tables are easy to get wrong.

### B. Cubie model used by solvers: 8 corners (position + twist 0..2), 12 edges (position + flip 0..1)
- (+) Compact; legality checks; the base of real solvers.
- (-) Hardest to explain; colors come from lookup tables; centers are not represented.

### C. 27 cubies, each with an integer position `(x, y, z)` in {-1, 0, 1} and an integer rotation matrix
A move = "rotate every cubie whose coordinate on that axis equals the layer by 90 degrees about the axis".
- (+) The same data drives logic and drawing; moves are geometry (the topic of the course), with no tables; animation falls out naturally.
- (-) Stickers are derived (position + rotation -> which color faces where); a solver would convert to B.

## Recommendation
C for the state, plus a function `to_facelets()` (54 colors) used for printing and tests.

## Answers
- Marcus:

- CristianMellado: Apoyo la Opción C (27 cubies con posición entera (x,y,z) en {-1,0,1} y matriz de rotación entera). Es la decisión correcta para un curso de computación gráfica porque los movimientos son rotaciones 3D reales, no tablas mágicas. La lógica es: rota todos los cubies cuya coordenada en el eje correspondiente sea igual a la capa, 90 grados alrededor de ese eje. Eso es una transformación matricial pura, que es exactamente el tema del curso. Además la animación cae de forma natural porque el estado ya vive en el espacio geométrico. La función to_facelets() resuelve el printing y los tests sin contaminar la estructura principal.

- FabricioMessa:

## Decision
