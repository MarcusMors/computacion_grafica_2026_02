# Q-007: How do we split the work among the three of us?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

## Context
Three people, three agent sessions, one repository. Conflicts are rare when each person owns different files and the interfaces
between them are written first.

## Options
### A. By layer, interfaces first
1. First pull request: headers with declarations only (`Face`, `Move`, `Cube_State`, `Rubik_Cube`, `Turn_Animation`) plus the ADRs.
2. Then in parallel: **state** (pure logic, tested without OpenGL by a small `rubik_state_test.cpp`), **view** (figures, colors,
   transforms), **animation + input** (`main.cpp`, queue, camera, help text).
3. Final pull request: integration and polish together.
- (+) Few merge conflicts; each part is testable alone.
- (-) Needs agreement on the headers up front.

### B. Everyone works on everything (pair sessions)
- (+) Everybody learns every part.
- (-) Constant conflicts in the same files.

## Recommendation
A, and rotate the owners for the second half so everybody touches every layer. Names for the three roles are filled in the answers.
Folder: `glfw-master/OwnProjects/rubik_cube/`, with its own `CMakeLists.txt` copied from an existing project.

## Answers
- Marcus (which layer do you take?):

- CristianMellado:

- FabricioMessa:

## Decision
