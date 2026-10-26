# Q-007: How do we split the work among the three of us?

- **Topic:** rubik
- **Status:** open
- **Opened by:** Marcus's agent (facilitator), 2026-10-25
- **Needs an answer from:** Marcus, CristianMellado, FabricioMessa

## Context
Three people, three agent sessions, one repository. Conflicts are rare when each person owns different files and the interfaces
between them are written first. This question is about who WRITES code; who ANSWERS which question is in `START_HERE.md`.

## Options
### A. By layer, interfaces first
1. First pull request: headers with declarations only (`Face`, `Move`, `Cube_State`, `Rubik_Cube`, `Turn_Animation`) plus the ADRs.
2. Then in parallel: **state** (pure logic, tested without OpenGL by a small console program `tests/rubik_state_test.cpp`),
   **view** (figures, colors, transforms), **animation + input** (`main.cpp`, queue, camera, help text), and the **solver bridge**
   (`solve()` of Q-011: tiny if it replays the history, bigger if a library is wired in; whoever has the lightest part takes it).
   The project template compiles every `*.cpp` next to its `CMakeLists.txt` into ONE executable, so a test with its own `main()`
   must live in a subfolder with its own `add_executable` (about 3 lines: the first change to the template).
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
- Marcus (which layer do you take?): A. I agree with Cristian's layering for now: I take the State layer, Cristian the View, Fabricio Animation + Input.

- CristianMellado:

- FabricioMessa:

## Decision
