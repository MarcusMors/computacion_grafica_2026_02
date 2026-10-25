# Glossary: porting

| term | meaning |
|------|---------|
| **bundle** | the folder (or zip) a student receives for one operating system: sources + scripts, plus the portable tools that OS needs |
| **portable compiler** | a compiler that runs from inside the LabCompiler folder with no installation. Windows: WinLibs MinGW GCC. Not available as a bundle for Linux/macOS |
| **system compiler** | the compiler installed on the user's machine (apt, Homebrew) |
| **`#embed`** | C23 / C++26 preprocessor directive that includes a file's bytes at compile time. Our projects use it to put `.glsl` files into the program. Needs GCC 15+ or Clang 19+ (Apple Clang: no) |
| **`#embed` probe** | a tiny program the setup script compiles to find out whether the compiler accepts `#embed`, instead of trusting version numbers |
| **`Shader_Source`** | the `my_lib` class that holds shader text. Plan: accept an embedded literal OR the path of a `.glsl` file |
| **GL floor** | the lowest OpenGL version the library promises to run on. macOS stops at 4.1, so the floor for library shaders is 4.1 (`#version 410 core`) |
| **forward-compatible core context** | the only kind of OpenGL context macOS gives (`GLFW_OPENGL_FORWARD_COMPAT`) |
| **WinLibs** | the MinGW-w64 GCC build we ship on Windows (Brecht Sanders) |
| **xPack GCC** | portable GCC builds for Windows and Linux (x64, arm64), no macOS. 15.2.0-1 exists |
| **WSL / WSLg** | Windows Subsystem for Linux; WSLg lets Linux programs show windows on Windows. Our Linux test bed |
| **lab machine** | a university computer: Windows, no compiler, permissions restricted. Reason why the Windows bundle contains a compiler |
