# Q-008: What is in version 1 and what is left out?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa, before Q-002 and Q-006

## Context
A clear finish line keeps a three-person course project from sprawling.

**Fixed by the course** (relayed by Marcus, 2026-10-25), so these are not options:
- draw the cube; turn any of the three layers of each axis, the middle one too (Q-010), animated;
- scramble and reset;
- show the cube being solved: an animation from scrambled to solved, **every move displayed on screen** (where the solution
  comes from is Q-011).

"Displayed" means the name of the move being played, as text in the window: `Draw2D::text`, a built-in 5x7 pixel font. That font
has no apostrophe, so `R'` cannot be spelled yet. Adding the glyph is one line in the `Draw2D` table (a small library pull
request; this question is its "question first"). It is needed in every option below.

Q-002 (the state) and Q-006 (the keys) depend on this answer.

## Options (what comes on top of the fixed list)
### A. Only the fixed list
- (+) Achievable early; everything else can be added on top.
- (-) No way to take back a move; no key help on screen.

### B. A, plus comfort: undo / redo, a list of the moves played, an on-screen help
Undo is cheap: it plays a move's inverse.
- (+) Feels like a real program; little extra code.
- (-) A history structure to maintain.

### C. B, plus presentation controls: pause, step one move, faster / slower while the solution plays
The animation is a queue of moves (Q-005): pause = stop taking moves from the queue, step = take exactly one, faster = scale the time.
- (+) Useful in front of a class: you can stop on a move and explain it.
- (-) A few more states in the animation code.

## Recommendation
B now, and C once the solving demo works (it only touches the animation queue). The apostrophe glyph is part of every option.

## Answers
- Marcus: B

- CristianMellado:

- FabricioMessa:

## Decision
