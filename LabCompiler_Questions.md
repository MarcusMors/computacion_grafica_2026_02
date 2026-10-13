# LabCompiler — Questions Before Proceeding

> **Date:** 2026-08-25\
> **Context:** I read `LabCompiler_Analysis_and_Migration_Guide.md` and inspected the actual\
> `glfw-master/deps/` and `glfw-master/OwnProjects/` folders. Below is everything I need to\
> know to align the project with (a) your university classes and (b) the OpenGL book you are\
> following (chapters 1.1.3–1.1.7: GLFW, extension library, GLM, SOIL2, optional OBJ loaders).
>
> **How to answer:** replace each `> ANSWER:` line (or just reply in chat). My **recommendation**\
> is listed first so you can approve everything quickly with "go with your recommendations".

***

## Q1. Extension Loader — GLEW (book) Vs Glad2 (project)

* **The book uses GLEW** (OpenGL Extension Wrangler), installed from glew\.sourceforge.net.
* **Your project currently uses legacy glad1** — four duplicate ~150 KB `glad.c` files, one per\
  project, with a broken `DEPENDENCY_DIR` reference in the CMake files (documented in §5 of the guide).
* The guide recommends migrating to **glad2** (one central copy, header-only option, actively maintained).
* Note: GLEW and glad2 do the same job. GLEW is older and its last release is from 2021; glad2 is\
  what most current tutorials use. However, if your **professor/book exercises literally require\
  `#include <GL/glew.h>` and `glewInit()`**, following the book exactly may matter for grading.

**Options:**

* **(A) Migrate to glad2** (recommended) — modern, fixes the existing bug, one shared copy.\
  I can show you the 2-line code difference vs GLEW whenever an exercise uses `glewInit()`.
* **(B) Add GLEW as-is** — follow the book literally, keep glad untouched for now.
* **(C) Add both** — GLEW for book exercises, glad2 for class projects.

> ANSWER: option (A), migrate to glad2, i'll adjust the book's code to modern standards. However if there is little to no overlapping between both libraries, i would like to do both (C), with some boolean variables at beginning to active or deactivate them (if glad2 is in general the same with different naming convention, i mean it's what must be learned, pick A)

***

## Q2. Texture Loading — SOIL2 (book) Vs stb\_image (already installed)

* **The book uses SOIL2** (updated fork of SOIL) for loading image files as textures.
* **Your project already ships `stb_image.h` and `stb_image_write.h`** in `glfw-master/deps/` —\
  header-only, zero build steps, supports JPEG/PNG/TGA/BMP/etc. LearnOpenGL (the samples already\
  in your `deps/learnopengl/`) uses stb\_image too.
* SOIL2 requires compiling a static library and wiring it into CMake; its API is\
  `SOIL_load_OGL_texture(…)` which also creates the OpenGL texture for you (convenient but hides what the book's chapter 5 wants you to learn: `glTexImage2D`, filtering, mipmaps).

**Options:**

* **(A) Keep stb\_image** (recommended) — I adapt book code (`SOIL_load_OGL_texture` →\
  `stbi_load` + `glTexImage2D`) whenever an exercise uses SOIL2.
* **(B) Also build SOIL2** — follow the book literally; I download SOIL2 source, build it with\
  your MinGW, and wire it into CMake.

> ANSWER: I would like to keep both, i want the one who has better compatibility with c++ and modern c++. If SOIL2 can't be adapted to be portable in a usb (we have a compiler in the usb, is it really incompatible?, if so just keep stb\_image)

***

## Q3. GLM — Already Present, Version Check

Good news: `glfw-master/deps/glm/` already exists, so the book's §1.1.5 requirement is covered.

* **Question:** Do you want me to **check the bundled GLM version** and upgrade it to the latest\
  (1.0.x) if it's old? Newer GLM is still header-only, so it's a drop-in folder replacement.

**Options:**

* **(A) Check version, upgrade only if outdated** (recommended).
* **(B) Leave GLM exactly as-is.**

> ANSWER: (A) Check version, upgrade only if outdated

***

## Q4. Modernization Tasks From the Guide — Which Ones Do I Apply Now?

Section 7 of the guide lists pending tasks. Which subset should I actually execute?

| Task                                                             | My take                                               |
| :--------------------------------------------------------------- | :---------------------------------------------------- |
| glad1 → glad2 migration                                          | Depends on Q1                                         |
| **Enable C++26** (`CMAKE_CXX_STANDARD 17 → 26`)                  | Recommended — GCC 14.2 already supports it, zero risk |
| **Fix broken `DEPENDENCY_DIR` reference** in project CMake files | Recommended — it's a latent bug                       |
| Upgrade CMake 3.27 → newer                                       | Optional, low value                                   |
| Upgrade MinGW GCC 14.2 → newer                                   | Optional, low value (14.2 is very recent)             |
| Upgrade GLFW master snapshot → stable 3.4 release                | Optional — current one works                          |

**Options:**

* **(A) C++26 + fix DEPENDENCY\_DIR only** (recommended minimal set).
* **(B) All of the above** (including downloading newer CMake/MinGW — big downloads).
* **(C) Custom** — tell me which.

> ANSWER: B, i heard about a miniature version of mingw in case the compiler is too big to fit in a 64 GB usb (consider the files will take up to 8GB at max)

***

## Q5. Where Do Book Exercises Live?

The guide shows projects registered in `glfw-master/OwnProjects/CMakeLists.txt`

(`Project_00` … `Project_03`, `hw0-glfw`).

**Options:**

* **(A) Continue the numbering** — book exercises become `Project_04`, `Project_05`, … inside\
  `OwnProjects/` (recommended; everything builds with the same batch file).
* **(B) Separate folder** — e.g., `glfw-master/BookExercises/Chapter_02/`, registered the same way.
* **(C) Use `hw0-glfw` as a template** — I'll clone it per exercise.

> ANSWER: A

***

## Q6. Internet Downloads — Allowed?

To follow the book/migration I may need to download:

* glad2 generated files (via gen.glad.sh or `pip install glad2`) — small
* GLEW source/binary (if Q1 = B or C) — \~3 MB
* SOIL2 source (if Q2 = B) — \~2 MB
* Newer CMake / MinGW (only if Q4 = B) — \~100 MB / \~1.5 GB

**Options:**

* **(A) Yes, download what's needed into the project folder** (recommended).
* **(B) No downloads** — work only with what's already on disk (rules out GLEW and SOIL2).

> ANSWER: I would like this project to be quite maintainable, so you can create another executable that downloads and sets the compiler, libraries, etc. With boolean variables at the beginning so the user can pick what it needs to be upgraded/downloaded from the latest version. So option A with an aditional executable called `_DOWNLOAD_LATEST_` or if you find a better name do so.

***

## Q7. Class Vs Book Priority When They Conflict

You said this project serves two purposes. If a class assignment and a book exercise ever

conflict (e.g., different OpenGL versions, different libraries), which takes precedence?

**Options:**

* **(A) Class assignments take priority; book exercises adapt.**
* **(B) Book takes priority.**
* **(C) Keep both fully working side-by-side** (recommended — costs nothing given the\
  per-project CMake structure).

> ANSWER: C

***

## Summary of My Default Plan (if You Just Say "go with recommendations")

1. Migrate extension loading to **glad2** (delete 4 duplicate `glad.c`, one central copy, fix the\
   `DEPENDENCY_DIR` bug). Book code using GLEW gets a 2-line adaptation.
2. Keep **stb\_image** for textures; adapt book's SOIL2 calls.
3. Check GLM version; upgrade only if outdated.
4. Enable **C++26** everywhere (root + subprojects, inheriting from root).
5. Book exercises go into `OwnProjects/Project_04+` using `hw0-glfw` as the template style.
6. Download only glad2 files (\~a few hundred KB).
7. Verify everything still builds by running the batch-file flow\
   (`_SET_PATHS_x64.bat` → `_GENERATE_MINGW_x64.bat`) and report the result.

> If anything above is wrong, correct it inline and I'll adjust before touching any files.

