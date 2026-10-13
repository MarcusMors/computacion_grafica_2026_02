Project_12_3D_figures_animated  -  C++17, self-contained
=========================================================

A castle made of boxes and pyramids; its four corner towers jump, each one
starting a little after the previous one.

HOW TO BUILD (inside LabCompiler)
  1. Copy this whole folder into  glfw-master/OwnProjects/
  2. Add this line to  glfw-master/OwnProjects/CMakeLists.txt :
         add_subdirectory( Project_12_3D_figures_animated )
  3. Run _SET_PATHS_x64.bat, then glfw-master/_GENERATE_MINGW_x64.bat as usual.
  4. The program is  glfw-master/bin/Project_12_3D_figures_animated.exe

The CMakeLists.txt of this folder selects C++17 for this project by itself.
Nothing outside this folder is needed except GLFW (already in LabCompiler).
OpenGL 4.3 core profile is requested (my_lib/_cpp.cpp, request_opengl_version).

CONTENTS
  main.cpp        window, main loop, keyboard, the jump animation (display())
  Castle.hpp      Tower and Castle, built from boxes and pyramids
  my_lib/         the small library used by the program:
                    Mat4.hpp / Transform.hpp   4x4 matrices, translate/rotate/scale
                    Camera.hpp                 fly camera
                    Mesh.hpp / Shapes.hpp      geometry: box, pyramid, sphere, 2D shapes
                    Figure.hpp                 Figure, Box, Pyramid, ..., Composite
                    Renderer.hpp               shaders (embedded as text) + drawing
                    Animation.hpp              pulse(), spin(), hop(), heartbeat()
                    Buffers.hpp, _cpp.hpp/.cpp OpenGL wrappers (VAO/VBO/EBO, Program)
                    Colors.hpp                 named colors (colors::stone, ...)
  third_party/glad/gl.h   OpenGL function loader (header only)

KEYS
  W A S D        move             Left Shift / Left Ctrl   up / down
  Arrow keys     look around      Esc                      quit
  O              outline on/off   T / R                    add / remove a tower
