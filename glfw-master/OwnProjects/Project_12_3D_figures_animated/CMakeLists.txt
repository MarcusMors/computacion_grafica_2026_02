# ============================================================
# Project_12_3D_figures_animated  -  SELF-CONTAINED, C++17
#
# Everything the program needs is inside this folder:
#   main.cpp, Castle.hpp      the program
#   my_lib/                   the library files (no outside my_lib needed)
#   third_party/glad/gl.h     OpenGL loader (header only)
# The only outside dependency is GLFW.
#
# Use 1 (inside LabCompiler): copy this folder into glfw-master/OwnProjects/
#   and add   add_subdirectory( Project_12_3D_figures_animated )
#   to glfw-master/OwnProjects/CMakeLists.txt. The exe goes to glfw-master/bin.
# Use 2 (alone, GLFW installed):  cmake -S . -B build && cmake --build build
# ============================================================
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    cmake_minimum_required(VERSION 3.10)
    set(STANDALONE TRUE)
endif()

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project( ${PROJECT_NAME} C CXX )

if (NOT STANDALONE)
    # Put the .exe in glfw-master/bin next to the other projects
    set(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin)
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# GLFW: already a target when built inside LabCompiler, otherwise look for it.
if (NOT TARGET glfw)
    find_package(glfw3 REQUIRED)
    add_library(glfw ALIAS glfw3::glfw)
endif()

set(SOURCES
    main.cpp
    my_lib/_cpp.cpp
)
file(GLOB HEADERS "*.hpp" "my_lib/*.hpp")

if (WIN32)
    # use the same icon as the other projects when built inside LabCompiler
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../glfw.rc")
        set(ICON ../glfw.rc)
    endif()
endif()

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${ICON})

# C++17 for this project, whatever the rest of the tree uses.
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

# BEFORE: our own glad/gl.h and library headers win over any other copy on the machine.
target_include_directories(${PROJECT_NAME} BEFORE PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/my_lib
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

target_link_libraries(${PROJECT_NAME} glfw)

if (MSVC OR CMAKE_C_SIMULATE_ID STREQUAL "MSVC")
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS NOMINMAX)
endif()

# MinGW: console+window subsystems, and STATIC libstdc++/libgcc/winpthread
# so the exe runs on any PC without MinGW DLLs on PATH
if (WIN32 AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_options(${PROJECT_NAME} PRIVATE "-mconsole" "-mwindows"
                        -static-libgcc -static-libstdc++ -static)
endif()
