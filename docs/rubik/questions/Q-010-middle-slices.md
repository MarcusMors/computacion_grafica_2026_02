# Q-010: Do we turn the middle slices (M, E, S) in version 1?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa, before Q-003

## Context
A cube has three layers per axis. The 18 standard moves (R, L, U, D, F, B x 3 amounts) only turn the two OUTER layers of each
axis. The middle layer is turned by **M** (turns like L), **E** (like D) and **S** (like F): 9 more moves.

**Course requirement (Marcus, 2026-10-25): the middle layer must turn too.** Option A stays on the page only so the choice is
visible; Cristian and Fabricio confirm in their lines.

What changes when the middle layer moves:
- The centers move (M carries the back center to the top), so "solved" means every face is one color, not "the standard color
  scheme". Only a freshly reset cube has U white, F green, and so on.
- Solvers assume the standard centers, so we rename colors by the centers before calling one (Q-011).
- With Q-002 C the state code is the same either way ("turn the cubies whose coordinate on that axis is -1, 0 or +1").
- Whole-cube turns (x, y, z) are not part of this question; the orbiting camera (Q-006) is how you look at the other sides.
  Q-012 may add them to show a solution.

## Options
### A. Face turns only (18 moves)
- (+) Fewer concepts. The six centers never change place (they only spin), so the color of a face is the color of its center.
- (-) Does not meet the course requirement. Patterns that use slices (e.g. the checkerboard `M2 E2 S2`) cannot be typed.

### B. Face turns and slices (27 moves)
- (+) Meets the requirement; every position of the puzzle is reachable by moves; complete notation.
- (-) M, E, S turn in the direction of L, D, F (not "clockwise seen from outside"): a quirk to explain. Centers move, so a face
  is no longer named by the color of its center; three more keys.

## Recommendation
B, because the course asks for it. The extra cost is small: the `Face` list gets three entries and the layer coordinate 0 is used.

## Answers
- Marcus: B. The middle layer must turn too (course requirement, said by Marcus in chat on 2026-10-25).

- CristianMellado:

- FabricioMessa: B — Caras + capas medias (27 movimientos); cumple el requisito del curso y el estado no cambia, solo añade M/E/S y la capa 0.

## Decision
