# Q-005: How do we animate one layer turn?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

## Context
While a layer turns, 9 cubies move together around the layer's axis and the other 18 stay still. When the turn ends the state
must change exactly (no accumulated float error). Depends on Q-002 and Q-004.

## Options
### A. Pivot figure: move the 9 cubies into a temporary `Composite` and rotate that
- (+) Conceptually simple ("a spinning layer").
- (-) `Composite` cannot move a figure from one parent to another today (`add()` copies it in); needs a library change.

### B. Keep all 27 cubies flat; while animating, draw the 9 moving ones with `R_layer(angle) * cubie_matrix`
At the end, apply the exact move to the state and stop animating.
- (+) No library change; plain matrix multiplication, which is the point of the course.
- (-) The view layer must know which cubies belong to the turning layer.

## Recommendation
B. State machine: `Idle` -> `Turning{ move, progress 0..1 }` -> `Idle`, with a queue of pending moves and one easing function (smoothstep).

## Answers
- Marcus:

- CristianMellado: Acuerdo total con la Opción B. El flujo es: mientras la animación está en progreso, los 9 cubies de la capa se dibujan con R_layer(progress * 90°) * cubie_transform, y los 18 restantes con su transform normal. Al terminar (progress >= 1.0) se aplica el move exacto al estado y se vuelve a Idle. Esto es multiplicación matricial pura, que es el tema del curso, y evita errores acumulativos de floating-point porque el estado final es exacto. La máquina de estados queda: Idle → Turning{ move, progress 0..1 } → Idle, con una cola de moves pendientes y smoothstep como función de easing.

- FabricioMessa:

## Decision
