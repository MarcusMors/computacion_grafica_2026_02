# Q-009: How does the view draw a cubie that is turned, if `Transform` stores Euler angles?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa, after Q-002 and Q-004, before Q-005

## Context
Found by checking Q-002 C, Q-004 and Q-005 B against `my_lib`; nobody decided it silently.

- Vocabulary: **Euler angles** describe a rotation as three successive turns (yaw about Y, pitch about X, roll about Z). When the
  middle turn (pitch) reaches 90 degrees, two of the three axes line up and one degree of freedom is lost ("gimbal lock"); that is
  why the library keeps pitch below 89. A **rotation matrix** has no such problem: it is the 3x3 block inside the 4x4 *model
  matrix* that the GPU multiplies into every vertex.
- After many turns a cubie can be in any of **24 rotations** (the 24 ways to put a die on a table). In the state it is an
  integer matrix, exact (Q-002 C).
- `Transform` stores Euler angles (`rotation_deg`) and clamps the pitch to +/-89 degrees (`MAX_PITCH_DEG`, a gimbal-lock guard).
  I checked all 24 rotations: **8 of them need pitch +/-90**. Those 8 would be drawn 1 degree off (a visible tilt); the other
  16 are exact. Project_13 already works around this limit for its ground slab.
- `Composite::draw` draws every part with the same parent matrix and its part list is private, so a `Composite` cannot draw
  "9 of its parts with one extra turn" (Q-005 B).
- `Figure::draw(renderer, parent)` accepts ANY `Mat4` as the parent, so a figure whose `Transform` stays at identity can be
  drawn in any pose.
- Word check: the glossary's **orientation** means the twist 0..2 of a solver cubie (Q-002 B). Here I say **rotation**
  (one of 24) and **pose** (position + rotation).

## Options
### A. `Rubik_Cube` is a plain `Figure` that owns the 27 `Box`es and draws each one with its own matrix
```cpp
void Rubik_Cube::draw(Renderer &r, const Mat4 &parent) const {
  for (int i = 0; i < 27; ++i)
    boxes_[i].draw(r, parent * pose_matrix(state_.cubies[i]));   // each Box's own Transform stays identity
}
```
The state hands out each cubie's pose as a `Mat4` whose entries are exactly 0, 1 or -1. During a turn, the 9 turning cubies are
drawn with `layer_rotation * pose_matrix(...)` (Q-005 B).
- (+) No `my_lib` change; exact for all 24 rotations; plain matrix multiplication, the topic of the course; one loop explains
  both the still picture and the animation.
- (-) `Rubik_Cube` does not use `Composite` (we lose `add()`/`remove()` and the free centroid/bounds, which we do not need);
  the `Transform` of each `Box` is unused, which may surprise a reader (one comment explains it).

### B. Teach `Transform` an exact rotation: an optional `Mat4` that replaces the Euler angles (a library pull request)
```cpp
std::optional<Mat4> rotation_override;   // when set, rotation_matrix() returns it
```
- (+) Cubies stay normal `Composite` parts with a `Transform`; the animation assigns position and rotation every frame, like
  the other animations in the library.
- (-) Changes a class every student already knows (two ways to rotate one object); a library change that touches other projects;
  during a turn the 9 cubies' transforms must be recomputed by hand (still easy).

### C. Keep Euler angles in the cubie and accept the clamp
- (+) No new concept.
- (-) 8 of 24 rotations drawn 1 degree off, and the angles must be computed from the integer rotation. Listed so the choice
  is visible; not recommended.

## Recommendation
A. Under our tiebreaker: a cubie "is" a matrix, no library class changes, and the loop above is the whole story.

## Answers
- Marcus: A, as long as the camera stays safe from gimbal lock and the cube gets exact full 90 degree turns.

- CristianMellado:

- FabricioMessa:

## Decision
