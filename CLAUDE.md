# LabCompiler - read this first (instructions for AI agents)

LabCompiler is a portable OpenGL teaching environment for a Computer Graphics course: C++26, GLFW, glad2 (header-only),
Boost.Rational and our own header-only library `my_lib/`. Its purpose is **learning**: code must stay readable and easy
for a student to modify.

Two independent topics live in this repository. Each has its own documentation folder:

| topic    | docs            | what it is                                                                  |
|----------|-----------------|-----------------------------------------------------------------------------|
| porting  | `docs/porting/` | make LabCompiler usable on Windows, Linux and macOS                         |
| rubik    | `docs/rubik/`   | a group project (3 students): a Rubik's cube in OpenGL, built on `my_lib`   |

How the humans work together is explained in `docs/README.md`.

## Rules for every agent

1. **The docs are the shared memory.** You do not remember other people's sessions. Before proposing or changing
   anything on a topic, read its `GLOSSARY.md`, every file in its `adr/` (accepted decisions: follow them) and the open
   files in its `questions/`.
2. **Never decide silently.** If something is undecided and it matters (data structures, public API, folder layout,
   dependencies, dropping a feature), write a question file from `docs/templates/QUESTION.md` and ask the human.
3. **One facilitator per topic.** Only the facilitator's agent creates question files (the human tells you who the
   facilitator is; for now it is Marcus's agent). Every other agent helps its own human *answer*: it edits ONLY that
   person's line under "Answers". Never edit someone else's answer.
4. **Decisions become ADRs** (`docs/templates/ADR.md`) only when the humans agree. Number them in sequence. Never rewrite
   an accepted ADR: supersede it with a new one.
5. **Small PRs.** One concern each, linked to its question or ADR. No direct pushes to `main`. Do not commit or push
   unless your human asks you to. Commit messages follow Conventional Commits (`feat:`, `fix:`, `docs:`, `refactor:`,
   `build:`).
6. **Teaching first.** Prefer the obvious solution over the clever one. Comments explain *why*, in the same tone as the
   existing headers. Do not add features nobody asked for. If a change makes the library harder to read, ask first.

## Layout

```
my_lib/                      our header-only library (Camera, Mat4, Transform, Mesh, Figure, Renderer, Draw2D, ...)
glfw-master/                 GLFW + deps; our own projects are in glfw-master/OwnProjects/<Project_name>/
glfw-master/OwnProjects/CMakeLists.txt   which projects are built (add_subdirectory lines)
docs/                        questions, decisions (ADRs) and glossaries, one folder per topic
_SET_PATHS_x64.bat, _GENERATE_MINGW_x64.bat   Windows environment + build scripts
```

## Building (today: Windows)

From the repository root in CMD: `_SET_PATHS_x64.bat`, then `cd glfw-master` and `_GENERATE_MINGW_x64.bat`.
Executables go to `glfw-master/bin/`; `bin\_RE_COMPILE_x64.bat` rebuilds. Linux and macOS are not ready yet
(see `docs/porting/`). The portable compiler, CMake, editor and Boost live in folders that are NOT in git.

## Code conventions

- Formatting: `.clang-format` (2 spaces, 120 columns, braces on their own line only for classes, structs and functions;
  includes are never sorted because glad must come before GLFW), `.editorconfig`, `.gitattributes` (LF everywhere,
  `.bat` / `.ps1` CRLF). Run clang-format only on the lines you changed.
- `my_lib` naming: classes `Capitalized_Words` (`Vertex_Array`, `Mesh_Figure`), functions and variables `snake_case`,
  constants `UPPER_CASE`, colors through `colors::name`. Headers are self-contained and mostly header-only.
- Figures own GPU objects: create them after the OpenGL context exists (in `init()`), destroy them before
  `glfwDestroyWindow`.
- New shaders use `#version 410 core` (the macOS maximum, see `docs/porting/adr/0003-opengl-version-policy.md`).
- New projects: copy an existing folder under `glfw-master/OwnProjects/`, rename it, add it to `OwnProjects/CMakeLists.txt`.

## Do not touch, do not commit

The repository holds only our code (ADR-0005): a bare `git clone` is not buildable, third-party code and tools come from the LabCompiler zip.

`glfw-master/src`, `include`, `deps` (except `deps/glad`), the compiler / CMake / editor / Boost folders, `bin/`,
`build/`, any `.exe`, `.o`, `.zip`. Do not edit another person's project folder without asking them.

## Verifying your work

Build the project you touched and run it. If you cannot run it (no GPU, headless), say so instead of claiming it works.
State which OS you tested on.
