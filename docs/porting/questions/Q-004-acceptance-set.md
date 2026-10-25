# Q-004: Which projects must build and run on Linux (and macOS)?

- **Topic:** porting
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus

## Context
`glfw-master/OwnProjects/CMakeLists.txt` enables only the recent projects; the older ones (Project_00 ... Project_09, Control_1,
hw0-glfw with Windows FreeImage libraries) are commented out.

## Options
### A. The enabled projects
Project_10 to Project_13, `pedagogic_visualizations_look_at` (and `_revised`), `max_opengl_supported`.
- (+) Clear "done" criterion.
- (-) The old ones may still rot.

### B. All of them
- (+) Complete.
- (-) `hw0-glfw` is tied to Windows binaries; much extra work for little value.

## Recommendation
A. The old projects are best effort and fixed when someone needs them.

## Answers
- Marcus:

## Decision
