# Q-003: How is a move represented, and can we type `R U R' U2`?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

## Context
Moves are used by input (keys), scramble, undo, animation and tests. The standard notation has 6 faces x 3 amounts = 18 moves.

## Options
### A. An enum with 18 values (`R`, `R_prime`, `R2`, ...)
- (+) Easy to read in a `switch`.
- (-) 18 cases everywhere; the inverse and the angle need tables.

### B. A small struct `Move { Face face; int quarter_turns; }` with `quarter_turns` in {1, 2, 3}
`inverse()` = `4 - quarter_turns`; the angle is `quarter_turns * 90` degrees; `parse("R U R' U2")` and `to_string()` for tests.
- (+) Three lines of logic; tests can be written as text.
- (-) A value like 0 or 5 must be refused.

## Recommendation
B.

## Answers
- Marcus:

- CristianMellado:

- FabricioMessa:

## Decision
