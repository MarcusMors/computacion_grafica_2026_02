# Q-001: 3x3x3 only, or any size NxNxN?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa, before Q-002

## Context
Everything else (state, moves, drawing) depends on this. The standard puzzle is 3x3x3. Smaller (2x2x2) and bigger (4x4x4) cubes exist.

## Options
### A. 3x3x3 only
- (+) Smallest problem; the notation (`R`, `U'`, `F2`) is fixed and well known.
- (-) Hard to extend later.

### B. 3x3x3 first, but the size is one constant `N` and code avoids hard-coding 3 where that costs nothing
- (+) A 2x2x2 comes almost for free; shows good structure.
- (-) Slightly more abstract code; 4x4+ needs extra notation (inner slices) that we do NOT build.

## Recommendation
B, with the promise that only N = 3 is tested and supported.

## Answers
- Marcus:
- CristianMellado:
- FabricioMessa:

## Decision
