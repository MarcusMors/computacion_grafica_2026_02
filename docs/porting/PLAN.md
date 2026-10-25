# LabCompiler: porting to Linux (Ubuntu) and macOS - decisions, facts, plan

Status: **PLAN, nothing implemented yet.** Written after the questions were answered (see "Decisions").

## 1. Goal (as the owner stated it)
LabCompiler stays a *learning* tool: one readable source tree, shipped as a folder per operating system, compiled **on the
user's own machine** after they set up their PATH/settings. Different script files per OS (`.bat` / `.ps1` on Windows,
`.sh` on Linux and macOS). **No universal binary.** Readability and maintainability win over cleverness.

## 2. Decisions (from the owner)
| # | Decision |
|---|---|
| D1 | Linux/macOS use the **system compiler**, but only if it supports `#embed`. If it does not, the user installs one that does (we cannot reuse the Windows MinGW bundle there). |
| D2 | `#embed` support is **mandatory** for the projects that use it. Fallback for people who cannot: paste the GLSL text into a string literal, **and** make `Shader_Source` accept either a **file path** or a **literal** (see 5.3). |
| D3 | Explore lowering the OpenGL floor to **4.1** (macOS maximum) and what it means for `my_lib` (see 4). |
| D4 | Mac friends from the class will test; no Mac is available to the owner or to Claude. |
| D5 | Linux laptop: Ubuntu 26.04 LTS, Intel UHD 620 (+ NVIDIA MX110 with no driver loaded), **Wayland** session. WSL2 Ubuntu 24.04 on the Windows machine is the build/test bed. |
| D6 | Defaults for the rest: build the bundled `glfw-master` from source on every OS; transfer by zip (Google Drive) or git. |

## 3. Facts checked (not from memory)
* **`#embed` in GCC:** listed in GCC 15's changes under C++26 ("P1967R14, `#embed`") and C23 - <https://gcc.gnu.org/gcc-15/changes.html>.
  *Tested here:* GCC 16.1 (WSL) and GCC 16.2 (the Windows bundle) accept it in every `-std` from c++17 to c++26;
  **GCC 14 (Ubuntu 24.04's own) rejects it** (`invalid preprocessing directive #embed`). GCC 15 is **not tested yet**.
* **`#embed` in Clang:** added in Clang 19 - <https://www.phoronix.com/news/LLVM-Clang-19-C23-Embed>.
* **Apple Clang:** Xcode 16.3 through 26.x ship "Apple clang 17.0.0"; no source found that it includes `#embed` (upstream has it since 19),
  so assume **no** until a Mac says otherwise. Routes on macOS: Homebrew `gcc` (15/16) or Homebrew `llvm` (19+).
* **Ubuntu 26.04** ships GCC 15.2 as default (press coverage, to confirm with `gcc --version` on the laptop) -
  <https://linuxconfig.org/ubuntu-26-04-release-date-and-new-features-in-resolute-raccoon>.
* **macOS OpenGL:** maximum 4.1, core profile only, and it needs `GLFW_OPENGL_FORWARD_COMPAT = GLFW_TRUE` -
  <https://castle-engine.io/wp/2023/02/22/using-opengl-core-profile-by-default-on-macos/>. macOS clamps `glLineWidth` to 1.
* **Repo scan:** no code or GLSL in `my_lib` or `OwnProjects` uses anything above OpenGL 4.1 (no compute, SSBO, image load/store,
  `layout(binding)`, explicit uniform locations, debug output). 4 `#version 330`, 6 `330 core`, 17 `430`, 29 `430 core` lines exist.
  No Windows-only code outside CMake and the vendored `stb_image.h` / `glad` headers. No `#include` case mismatches (Linux is case-sensitive).
* **Size:** sources + `glfw-master` without build/bin = ~39 MB; `Boost/` headers = 191 MB (optional: system `libboost-dev` / `brew install boost`);
  `MinGW/`, `Cmake/`, `Editor/` are Windows-only and not needed.
* **WSL test bed:** Ubuntu 24.04, GCC 16.1 (custom) + gcc-14, CMake 4.3.2, Ninja, GLFW/X11/Wayland dev packages, WSLg display. No passwordless sudo.

## 4. What lowering to OpenGL 4.1 means
* **`my_lib` itself: no functional loss.** `Draw2D` and `Renderer` shaders only need `layout(location)` on vertex inputs (GLSL 3.30+) -> change their
  `#version 430 core` to `410 core`. `Buffers.hpp`, `Program`, `Draw2D` use GL 3.x calls only.
* **`request_opengl_version()`:** becomes 4.1 core + forward-compat (harmless on Windows/Linux). Optional parameters let a lab that really needs 4.3
  ask for it, documented as "not on macOS".
* **Lab projects:** their `#version 430` lines would fail on macOS (GLSL 4.30 unsupported) although they use nothing above 4.10. Options:
  (a) edit them to 410, (b) let `Program::attach` rewrite `#version NNN` down to the context's GLSL version and print one notice line.
* **Student loss:** compute shaders, SSBOs, image load/store, `layout(binding=)`, KHR_debug are unavailable on macOS anyway.
* **macOS extras:** `glLineWidth(2)` in `Renderer` becomes 1 px (cosmetic); Retina framebuffers are larger than the window (the code already uses
  `glfwGetFramebufferSize`); Project_00 requests a 3.x context (needs forward-compat on macOS).

## 5. Plan
### 5.1 Repository hygiene
`.gitattributes` (`*.sh text eol=lf`), `.gitignore` for new per-OS outputs, note that zips from Windows drop the executable bit (`chmod +x *.sh` or `bash script.sh`).
### 5.2 CMake
Per-project template: MinGW-only flags (`-mconsole -mwindows -static`, `--allow-multiple-definition`) and `WIN32` only on Windows; a configure-time
**`#embed` probe** (`check_cxx_source_compiles`) that stops with a clear message pointing to `_CHECK_ENV.sh`. Root: GLFW X11 + Wayland on Linux.
### 5.3 `my_lib`
library shaders at `#version 410`, lab shaders edited 430 -> 410; `request_opengl_version(major=4, minor=3)` with automatic 4.1 core + forward-compat on macOS; `Shader_Source(std::string_view)` decides **path or literal**: no newline / `{` / `;`, short, known shader extension
(`.glsl .vert .frag .vs .fs .geom .tesc .tese .comp`) -> read the file (cwd, then the executable's folder); **if it looks like a path but is not found: throw with the
path and cwd** (silently compiling a file name as GLSL gives a confusing error); anything else is a literal.
### 5.4 Scripts (per OS)
* `_CHECK_ENV.sh`: detects Linux/macOS, compiler `#embed` probe (compiles a 5-line file), CMake >= 3.20, GLFW dev packages, prints exact `apt` / `brew` commands.
* `_SET_PATHS.sh` (to `source`), `_GENERATE.sh` (mirror of `_GENERATE_MINGW_x64.bat`), `_RE_COMPILE.sh`, `_UPDATE_COMPONENTS.sh` (optional portable CMake from Kitware, glm, glad, SOIL2).
* Optional later: `_SNAPSHOT.sh` (sha256sum based), `_GIT.sh`.
### 5.5 Verification
1. WSL Ubuntu 24.04 with GCC 16.1: configure, build all projects, run a project under WSLg. **Negative test** with gcc-14: the probe must fail with the helpful message.
2. The owner's laptop (Ubuntu 26.04, GCC 15.2, Wayland): the same, plus the look-at viewer. Confirms GCC 15 `#embed`.
3. A Mac friend with a checklist (`_CHECK_ENV.sh` output, build log, screenshot). macOS stays "best effort, unverified" until then.

## 6. Resolved decisions and the one open question
**Resolved (owner's answers):**
1. Lab shaders: **edit `#version 430` to `410`** in the files (no run-time rewriting).
2. `request_opengl_version()`: **students explore their own hardware**. Default stays 4.3 (Windows/Linux), with optional `(major, minor)` arguments; on macOS only
   4.1 core + forward-compat can work, so the library applies it automatically and prints one notice line. `max_opengl_supported` stays as the exploration tool.
3. Path-or-literal `Shader_Source`: **throw** when a path-looking string is not found.
4. Boost on Linux/macOS: **system package** (CMake already falls back to it).
5. CMake: **provide it** - Kitware publishes official binaries for all three OSes (v4.4.3: linux-x86_64 65 MB, linux-aarch64 52 MB, macos-universal 89 MB,
   verified by HEAD request), so `_UPDATE_COMPONENTS.sh` can fetch it exactly like the Windows script. Optional toggle; system CMake >= 3.20 also works.
7. Newest compilers on the owner's own machines are fine. **The real constraint: some university lab machines have no compiler at all**, which is why a compiler is bundled.

**Compiler for machines with none (what exists for each OS):**
| Option | Windows | Linux | macOS | `#embed` | Self-contained? | Notes |
|---|---|---|---|---|---|---|
| WinLibs MinGW GCC 16.2 (current bundle) | yes | no | no | yes | yes (headers, import libs) | keep as is |
| conda-forge via micromamba (no root) | n/a | yes, GCC 15.2 confirmed | yes (Clang 21 default announced) | yes (GCC 15 / Clang >= 19) | mostly: also gives X11/Wayland/GL dev packages | ~1-2 GB in the folder; runtime GL driver libs vs. system ones is a known risk (UNTESTED); macOS still needs Xcode Command Line Tools (Apple SDK cannot be redistributed) |
| Zig (`zig c++` = Clang + libc++ + libc headers) | yes | yes | yes | needs Clang >= 19 (Zig's LLVM version not verified) | std library yes; X11/Wayland/GL headers NO | one ~100 MB download per OS; unusual to debug/maintain |
| Official LLVM/Clang release | yes | yes | yes | Clang 19 "partial" (cppreference `19*`, note unreadable) | NO (needs system libstdc++/libc headers) | not self-contained |
| xPack GCC 15.2.0-1 (released 2025-10-14) | yes (MinGW-w64 13, UCRT) | yes (x64 + arm64, needs glibc >= 2.28) | **no binaries** (Intel build deprecated, none for Apple Silicon) | yes (GCC 15) - to be verified with our probe | partly: Linux still needs system C headers (`libc6-dev`) and X11/Wayland/GL dev packages | source: <https://xpack-dev-tools.github.io/gcc-xpack/blog/2025/10/14/gcc-v15-2-0-1-released/> |
| Apple Clang 17 | - | - | system | **no** (cppreference: blank) | needs Xcode CLT | use Homebrew gcc/llvm |

cppreference (C++26 support table): `#embed` / P1967R14 = GCC 15, Clang 19*, EDG eccp 6.8; MSVC and Apple Clang blank. EDG eccp is the commercial C++ front end
used inside other vendors' compilers (it is not something the course needs).

**Key point:** on Linux a compiler alone is not enough - building GLFW also needs the X11/Wayland/GL development headers, which a bare lab machine lacks too.
So the options are (a) system packages (needs admin once), or (b) the no-root conda-forge route above.

**ANSWERED:** the lab machines without a compiler are **Windows** (permissions are painful there, hence the bundled compiler). Linux machines come with sudo, so
system packages are fine. Therefore: Windows keeps the WinLibs MinGW bundle (GCC 16.2, newer than xPack's 15.2); Linux uses the system compiler, with xPack GCC 15.2.0-1 as an
optional no-PPA download for distros without GCC 15 (Ubuntu 22.04/24.04); macOS uses Homebrew `gcc` or `llvm` (xPack has no macOS build).
Binaries built with xPack's GCC should link `-static-libstdc++ -static-libgcc` so they do not need its libstdc++ at run time.

## 7. Risks
GCC 15 `#embed` in C++26 mode is untested; Apple Clang 17 likely lacks `#embed`; Wayland may ignore some window calls (e.g. fixed aspect ratio) - cosmetic;
macOS OpenGL is deprecated but works; WSLg may report a lower GL version than real hardware; `.sh` files with CRLF break (`.gitattributes`); hybrid GPU laptops may need
the discrete GPU requested explicitly.
