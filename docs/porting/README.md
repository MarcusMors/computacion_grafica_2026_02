# Topic: porting LabCompiler to Linux and macOS

**Goal.** LabCompiler is a portable folder that lets students compile OpenGL code on any Windows 11 machine, with no
installation. We want the same experience on Linux (Ubuntu first) and macOS: one readable source tree, shipped per
operating system, compiled on the user's own machine after they run a setup script. No universal binary.

- **Facilitator:** Marcus (his agent writes the questions).
- **Who answers:** Marcus (this topic has one decision maker).
- **Test machines:** WSL2 Ubuntu 24.04 (GCC 16.1, builds here); Marcus's laptop, Ubuntu 26.04, Wayland, Intel UHD 620;
  macOS: friends from the class (nobody on the team has a Mac).

## Where we are

Nothing is implemented yet. The research and the first decisions are done:

- [`PLAN.md`](PLAN.md): the full plan with the facts we checked (compilers, `#embed`, macOS OpenGL, sizes).
- Accepted decisions: [`adr/`](adr/).
- Open questions: [`questions/`](questions/).

## Order of work (after the open questions are answered)

1. `.gitattributes`, `.editorconfig`, `.clang-format`, `.clang-tidy`, `CLAUDE.md`, docs (done).
2. CMake: MinGW-only flags only on Windows, an `#embed` compiler probe at configure time.
3. `my_lib`: shaders to 410, `request_opengl_version()` policy, `Shader_Source` accepting a path or a literal.
4. Scripts: `_CHECK_ENV.sh`, `_SET_PATHS.sh`, `_GENERATE.sh`, `_RE_COMPILE.sh`, `_UPDATE_COMPONENTS.sh`.
5. Verify: WSL with GCC 16 and with gcc-14 (must fail with a clear message), then the xPack GCC 15.2 spike, the laptop, then a Mac.
