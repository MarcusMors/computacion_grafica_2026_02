# Q-008: What is in version 1 and what is left out?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

## Context
A clear finish line keeps a three-person course project from sprawling. Items below, cheapest first.

## Options
### A. Minimal
Draw the cube, turn layers with the keyboard (animated), scramble, reset.
- (+) Achievable early; everything else can be added on top.
- (-) No way to take back a move.

### B. Minimal + comfort
A, plus undo / redo (cheap: a move's inverse), a list of the moves played, and an on-screen help.
- (+) Feels like a real program; little extra code.
- (-) A history structure to maintain.

### C. B + a solver
- (+) Impressive.
- (-) A project in itself (needs the cubie model of Q-002 B and a lot of tables); not required by the course.

## Recommendation
B. The solver is out of scope for version 1; if the group wants it later it gets its own topic folder.

## Answers
- Marcus:

- CristianMellado: Elijo la Opción B: Minimal + comfort. El undo/redo es casi gratis (guardar moves e invertirlos al revertir) y hace que la app se sienta como un programa real. El scope de V1 sería: dibujar el cubo, girar capas con animación, scramble, reset, undo/redo, historial de movimientos en pantalla y help en pantalla. Fuera de scope V1: solver (requiere algoritmos complejos como Kociemba), mouse dragging (requiere ray casting) y persistencia. El solver puede ser V2 si el grupo lo quiere después.

- FabricioMessa:

## Decision
