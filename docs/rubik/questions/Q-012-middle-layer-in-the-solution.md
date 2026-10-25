# Q-012: Does the solving animation show middle-layer turns, and how?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa, after Q-011

## Context
The teacher warned that probably no solver gives middle-layer moves (they output only the 18 face moves). Marcus's idea: when
the solution turns the two outer layers of one axis the same physical way, show it as ONE middle-layer turn.

**It is correct.** Turning the top and bottom layers the same way is the same as turning the whole cube and then turning the
middle layer back. In notation (x, y, z are whole-cube turns, like R, U, F):

| the solver gives | has the same effect as | |
|------------------|------------------------|---|
| `U D'`, `U' D`, `U2 D2` | `E`, `E'`, `E2` followed by `y`, `y'`, `y2` | top + bottom |
| `R L'`, `R' L`, `R2 L2` | `M`, `M'`, `M2` followed by `x`, `x'`, `x2` | right + left |
| `F B'`, `F' B`, `F2 B2` | `S'`, `S`, `S2` followed by `z`, `z'`, `z2` | front + back |

(The two moves touch different layers, so they can be played in either order: `D' U` counts too.)

So the cube we draw after `E` alone is the solver's cube held turned by a quarter. What happens to that leftover whole-cube turn
is the decision. Two limits to know: it only applies where the solver happens to put such a pair next to each other with the
same amount (some solutions have none, so the animation may show no middle layer at all), and `U D` (opposite directions) is
not reducible.

**Fact to confirm with the teacher (Marcus, one message):** must the solution animation itself contain middle-layer moves, or is it
enough that the cube can turn its middle layer by hand (Q-010) and that a solution is animated?

## Options
### A. Show the solution exactly as the solver gives it
The middle layer is used by the player, by the scramble, and in the "unscramble" of Q-011 A. Solver output is played as is.
- (+) Nothing extra to build or test.
- (-) A solver-made solution never shows the middle layer.

### B. Replace the pair by the middle-layer turn, then visibly turn the whole cube
`U D'` is played as `E` and then `y`: the middle layer turns, then the whole cube turns a quarter (animated like a layer, but with all 27
cubies). The cube we show is always exactly the solver's cube. `x`, `y`, `z` become three more entries of the move list (Q-003).
- (+) Simple to explain and to test (state after = state the solver expects); the middle layer appears; no renaming.
- (-) Two animations for two original moves: no time saved, the middle layer just appears. Three more moves to write.

### C. Replace the pair by the middle-layer turn and remember the leftover turn
Play only `E`. Keep the leftover whole-cube turn as a "frame" and rename the face letters of the moves that follow (a 6-entry table that
changes every time a turn is absorbed). The cube we show is the solver's cube held turned.
- (+) One animation instead of two.
- (-) The renaming is the kind of cleverness our tiebreaker avoids; the final cube is solved but its colors can be in a different
  order than the standard scheme; harder to test.

## Recommendation
Ask the teacher first. If the middle layer need not appear inside the solution: A. If it must (or it would be nicer): B, because the
displayed cube always equals the solver's cube. C only if B's extra whole-cube turns turn out to look bad in the demo.

## Answers
- Marcus:

- CristianMellado:

- FabricioMessa: A — Mostrar la solución tal cual; con Q-011=A la capa media ya aparece al deshacer M/E/S, y si el profesor exige que aparezca siempre se cambia a B.

## Decision
