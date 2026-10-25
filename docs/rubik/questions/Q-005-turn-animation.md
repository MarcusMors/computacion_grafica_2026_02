# Q-005: How do we animate one layer turn?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

## Context
While a layer turns, 9 cubies move together around the layer's axis and the other 18 stay still. When the turn ends the state
must change exactly (no accumulated float error). Depends on Q-002, Q-004 and Q-009. `Animation.hpp` only has time -> value
functions (`pulse`, `spin`, `hop`, `heartbeat`); there is no easing function yet. **Easing** maps the linear progress t in 0..1 to a
smoother one so the layer starts and stops gently; smoothstep is `3t^2 - 2t^3`.

The same queue plays the solution: the moves returned by `solve()` (Q-011) are pushed into it, and the name of the move being played is
drawn on the screen (Q-008).

## Options
### A. Pivot figure: move the 9 cubies into a temporary `Composite` and rotate that
- (+) Conceptually simple ("a spinning layer").
- (-) `Composite::add()` moves a figure into a new owner, but nothing takes a part OUT of one `Composite` and puts it into
  another (parts are `unique_ptr`s in a private list), so it needs a library change.

### B. Keep all 27 cubies flat; while animating, draw the 9 moving ones with `R_layer(angle) * cubie_matrix`
At the end, apply the exact move to the state and stop animating.
- (+) No library change; plain matrix multiplication, which is the point of the course.
- (-) The view layer must know which cubies belong to the turning layer, and must draw the cubies itself instead of through
  `Composite::draw` (see Q-009).

## Recommendation
B. State machine: `Idle` -> `Turning{ move, progress 0..1 }` -> `Idle`, with a queue of pending moves and one easing function
(smoothstep, three lines, written in `rubik_animation` rather than added to the library unless someone prefers a library PR).

## Answers
- Marcus:

- CristianMellado:

- FabricioMessa: B — Animar los 9 cubies con `R_layer(angle) * matriz_cubie` y aplicar el movimiento exacto al estado al terminar; `Idle`/`Turning` + cola + smoothstep, asumiendo Q-002=C, Q-004=A y Q-009=A.

## Decision
