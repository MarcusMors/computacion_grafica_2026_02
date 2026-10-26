# Q-001: 3x3x3 only, or any size NxNxN?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa, before Q-002

## Context
Everything else (state, moves, drawing) depends on this. The standard puzzle is 3x3x3. Smaller (2x2x2) and bigger (4x4x4) cubes exist.
Our tiebreaker is understandability over performance (see the rubik README).

## Options
### A. 3x3x3 only
The number 3 is written once, as a named constant, so nobody wonders where it comes from. Each axis has three layers, at
coordinates -1, 0 and +1 (Q-002 C).
- (+) Smallest problem; the notation (`R`, `U'`, `F2`) is fixed and well known; the middle layer exists, so the coordinates -1, 0, +1 read naturally.
- (-) Hard to extend later.

### B. 3x3x3 first, but the size is one constant `N` and code avoids hard-coding 3 where that costs nothing
- (+) Shows good structure.
- (-) A 2x2x2 does NOT come "almost for free": it has no middle layer, so the coordinates -1, 0, +1 of Q-002 C turn into
  -0.5, +0.5 (or the doubled numbers -1, +1 for 2x2x2 and -2, 0, +2 for 3x3x3). Every student who reads the code has to
  understand that generality for something we never test. 4x4+ also needs inner-slice notation that we do NOT build.

## Recommendation
A. I first wrote B, assuming the 2x2x2 was nearly free; checking it against Q-002 C's coordinates it is not, and under our
tiebreaker the simpler reading wins. Only N = 3 is built, tested and supported.

## Answers
- Marcus: B, mientras no añada complejidad, está bien que admita variabilidad en el tamaño del cubo. Porque a veces en el examen nos puede pedir que hagamos un cubo 2x2 o 2x3 o 4x2 una variación pequeña por molestar (al menos en la creación y animación del cubo, resolver esas variaciones queda absolutamente fuera de orden).

- CristianMellado:

- FabricioMessa:

## Decision
