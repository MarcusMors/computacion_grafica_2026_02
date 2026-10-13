# ============================================================
# LabCompiler project template
# C++26 is inherited from the root glfw-master/CMakeLists.txt.
# To create a new project: copy this folder, rename it, and
# change ONLY the project(...) line below.
# ============================================================
project( Project_02_glfw )

# Put the .exe in glfw-master/bin next to the other projects
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

file(GLOB SOURCES "*.cpp" "*.c")
file(GLOB HEADERS "*.h" "*.hpp")
file(GLOB SHADERS "*.vert" "*.frag" "*.vs" "*.fs" "*.glsl")

if (WIN32)
    set(ICON ../glfw.rc)
elseif (APPLE)
    set(ICON ../glfw.icns)
endif()

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SOURCES} ${HEADERS} ${SHADERS} ${ICON})

# Dependencies: glfw always; my_lib (user's shared library) when enabled
target_link_libraries(${PROJECT_NAME} glfw)
if (TARGET my_lib)
    target_link_libraries(${PROJECT_NAME} my_lib)
endif()

if (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

# MinGW: console+window subsystems, and STATIC libstdc++/libgcc/winpthread
# so the exe runs on any PC without MinGW DLLs on PATH (USB portability)
if (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${PROJECT_NAME} PRIVATE "-mconsole" "-mwindows"
                        -static-libgcc -static-libstdc++ -static)
endif()

# Copy asset folders next to the exe (raw copy: same name = same file)
foreach(assetDir textures models shaders images assets)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${assetDir}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                "${CMAKE_CURRENT_SOURCE_DIR}/${assetDir}"
                "${CMAKE_SOURCE_DIR}/bin/${assetDir}")
    endif()
endforeach()
