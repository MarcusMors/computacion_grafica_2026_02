project(my_lib LANGUAGES CXX)

# OpenGL::GL target (needed below); glad2 and glm are header-only.
find_package(OpenGL REQUIRED)

# ----------------------------------------------------------------------------
# Collect your library sources
# ----------------------------------------------------------------------------
file(GLOB MY_LIB_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

file(GLOB MY_LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
)

# If you prefer to list files explicitly (recommended once the project grows):
# set(MY_LIB_SOURCES
#     ${CMAKE_CURRENT_SOURCE_DIR}/my_renderer.cpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/my_shader.cpp
# )
# set(MY_LIB_HEADERS
#     ${CMAKE_CURRENT_SOURCE_DIR}/my_renderer.hpp
#     ${CMAKE_CURRENT_SOURCE_DIR}/my_shader.hpp
# )

add_library(my_lib STATIC ${MY_LIB_SOURCES} ${MY_LIB_HEADERS})

# ----------------------------------------------------------------------------
# Everyone who links my_lib gets access to its headers automatically,
# plus the shared dependency headers in glfw-master/deps (glad2, glm,
# stb_image, SOIL2 headers, etc.)
# ----------------------------------------------------------------------------
target_include_directories(my_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GLFW_SOURCE_DIR}/deps
)

# ----------------------------------------------------------------------------
# Boost headers: LabCompiler/Boost/boost_<version>/boost/...  (portable, like MinGW/ and Cmake/).
# The highest version found is used. SYSTEM = its warnings are not shown.
# ----------------------------------------------------------------------------
file(GLOB LABCOMPILER_BOOST_DIRS LIST_DIRECTORIES true "${GLFW_SOURCE_DIR}/../Boost/boost_*")
if (LABCOMPILER_BOOST_DIRS)
    list(SORT LABCOMPILER_BOOST_DIRS)
    list(REVERSE LABCOMPILER_BOOST_DIRS)
    list(GET LABCOMPILER_BOOST_DIRS 0 LABCOMPILER_BOOST_ROOT)
    message(STATUS "my_lib: Boost headers from ${LABCOMPILER_BOOST_ROOT}")
    target_include_directories(my_lib SYSTEM PUBLIC ${LABCOMPILER_BOOST_ROOT})
else()
    message(STATUS "my_lib: no Boost folder (LabCompiler/Boost/boost_*) - projects using <boost/...> will not compile")
endif()

# ----------------------------------------------------------------------------
# Link every dependency as PUBLIC so that any executable linking my_lib
# automatically pulls in all required symbols / headers.
#
#   glfw  - windowing (target from the parent GLFW build)
#   soil2 - image loading (target from deps/SOIL2, if enabled)
#   OpenGL::GL - system OpenGL library
#
# glad2 and glm are header-only: they need no link target, only the
# include directory above.
# ----------------------------------------------------------------------------
target_link_libraries(my_lib
    PUBLIC
        glfw
        OpenGL::GL
)

if (TARGET soil2)
    target_link_libraries(my_lib PUBLIC soil2)
endif()

# ----------------------------------------------------------------------------
# Platform-specific tweaks
# ----------------------------------------------------------------------------
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # On Linux, some linkers need -ldl for dynamic loading.
    target_link_libraries(my_lib PUBLIC ${CMAKE_DL_LIBS})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Silence common MSVC warnings for OpenGL C headers included from C++.
    target_compile_definitions(my_lib PUBLIC NOMINMAX _CRT_SECURE_NO_WARNINGS)
endif()
