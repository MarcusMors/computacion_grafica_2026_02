#ifndef ___CPP_H__
#define ___CPP_H__

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "Shader.hpp"
#include "Buffers.hpp"// Vec2 / Vec3 / Vec4 (uniform value types)

using namespace std;

// Defined in Mat4.hpp — only the declaration is needed here because
// set_uniform(name, Mat4) takes it by reference. The definition is required
// at the CALL site (and in _cpp.cpp), both of which include Mat4.hpp.
struct Mat4;

//template<int m, int n>
//struct Matrix{

//};

//struct M3{
//	vector<vector<float>> M(3,vector<float>(3,{}));
//};

struct Shader_Pipeline
{
  Shader_Source vertex{};
  Shader_Source tessellation_control{};
  Shader_Source tessellation_evaluation{};
  Shader_Source geometry{};
  Shader_Source fragment{};
};

enum GL_Draw : GLenum {
  // glDrawArrays works with vertex array objects (VAOs) and buffer objects (VBOs)—the vertex attributes must be
  // properly set up via glVertexAttribPointer and enabled.
  // NOTE: values MUST be the real GL constants (they are not 0,1,2,3...).
  POINTS = GL_POINTS,
  //
  LINE_STRIP = GL_LINE_STRIP,
  LINE_LOOP = GL_LINE_LOOP,
  LINES = GL_LINES,
  //
  TRIANGLE_STRIP = GL_TRIANGLE_STRIP,
  TRIANGLE_FAN = GL_TRIANGLE_FAN,
  TRIANGLES = GL_TRIANGLES,
  //
  LINES_ADJACENCY = GL_LINES_ADJACENCY,
  LINE_STRIP_ADJACENCY = GL_LINE_STRIP_ADJACENCY,//(for geometry shaders)
  //
  TRIANGLES_ADJACENCY = GL_TRIANGLES_ADJACENCY,
  TRIANGLE_STRIP_ADJACENCY = GL_TRIANGLE_STRIP_ADJACENCY,
  //
  PATCHES = GL_PATCHES// requires tessellation shaders + glPatchParameteri(GL_PATCH_VERTICES, n)
};

struct P_glDrawArrays
{
  GL_Draw mode{};
  GLint start_index{};
  GLsizei count{};
};



struct P_glDrawElements
{
  GL_Draw mode{};
  GLsizei count{};
  // Element type of the indices stored in the bound EBO. Only 3 legal values:
  //   GL_UNSIGNED_BYTE  (uint8_t,  max index 255)
  //   GL_UNSIGNED_SHORT (uint16_t, max index 65 535)
  //   GL_UNSIGNED_INT   (uint32_t, max index ~4.29e9)
  // It MUST match the integer type uploaded into the EBO. To have it deduced
  // automatically instead, use Index_Buffer<T> + Vertex_Array (Buffers.hpp).
  GLenum type{GL_UNSIGNED_INT};
  // Offset of the first index to read, in INDICES (not bytes).
  // (The raw glDrawElements takes a const void* byte offset; this wrapper
  // converts for you, so no void* in the public API.)
  GLintptr first_index{};
};

// for drawing with indeces use glDrawElements
void glDrawArrays_cpp(P_glDrawArrays P);
void glDrawElements_cpp(P_glDrawElements P);

template<size_t N> constexpr string_view embed_to_string(const unsigned char (&data)[N])
{
  // Cast the unsigned char array to const char* for string_view
  return string_view(reinterpret_cast<const char *>(data), N);
}

void request_opengl_version();

struct P_glfwCreateWindow
{// window settings
  int width{};
  int height{};
  string title{ "Awesome App Title" };
  GLFWmonitor *fullscreen_mode{};// nullptr for windowed mode
  GLFWwindow *resource_sharing{};// nullptr if no sharing
};

GLFWwindow *glfwCreateWindow(P_glfwCreateWindow window_settings);


class Program
{
public:
  GLuint id{ glCreateProgram() };

  void validate_id();
  Program() { validate_id(); }
  void attach(const Shader_Pipeline &pipeline);

  void use() const { glUseProgram(id); }
  ~Program()
  {
    if (id != 0) { glDeleteProgram(id); }
  }

  // ==========================================================================
  // Uniforms — the seamless way.
  //
  // BEFORE (raw OpenGL, 3 steps scattered across init() and display()):
  //     GLint loc = glGetUniformLocation(program.id, "u_scale");   // init
  //     program.use();                                             // frame
  //     glUniform1f(loc, value);                                   // frame
  //
  // NOW (one call, anywhere you have the program):
  //     program.set_uniform("u_scale", value);
  //
  // - set_uniform() binds the program for you (glUseProgram), so a uniform
  //   can never silently land on the wrong program.
  // - Locations are looked up once and CACHED per program, so calling
  //   set_uniform() every frame costs one hash lookup, not a GL round-trip.
  // - A typo'd or optimized-out uniform name warns ONCE (not every frame).
  // ==========================================================================

  // Location of a uniform, cached. Returns -1 (and warns once) if the name
  // does not exist in this program. Passing -1 to glUniform* is a no-op,
  // so a missing uniform degrades gracefully instead of crashing.
  GLint uniform_location(const std::string &name) const
  {
    const auto it = uniform_location_cache_.find(name);
    if (it != uniform_location_cache_.end()) { return it->second; }

    const GLint loc = glGetUniformLocation(id, name.c_str());
    if (loc == -1) {
      cerr << "warning: uniform \"" << name << "\" not found in program " << id
           << " (typo, or the shader does not use it)\n";
    }
    uniform_location_cache_.emplace(name, loc);
    return loc;
  }

  // ---- scalars -------------------------------------------------------------
  void set_uniform(const std::string &name, float v) const
  { use(); glUniform1f(uniform_location(name), v); }

  void set_uniform(const std::string &name, int v) const
  { use(); glUniform1i(uniform_location(name), v); }

  void set_uniform(const std::string &name, unsigned int v) const
  { use(); glUniform1ui(uniform_location(name), v); }

  void set_uniform(const std::string &name, bool v) const
  { use(); glUniform1i(uniform_location(name), v ? 1 : 0); }

  // ---- small vectors, given as components ----------------------------------
  void set_uniform(const std::string &name, float x, float y) const
  { use(); glUniform2f(uniform_location(name), x, y); }

  void set_uniform(const std::string &name, float x, float y, float z) const
  { use(); glUniform3f(uniform_location(name), x, y, z); }

  void set_uniform(const std::string &name, float x, float y, float z, float w) const
  { use(); glUniform4f(uniform_location(name), x, y, z, w); }

  // ---- small vectors, given as the project's Vec types ---------------------
  void set_uniform(const std::string &name, const Vec2 &v) const
  { set_uniform(name, v.x, v.y); }

  void set_uniform(const std::string &name, const Vec3 &v) const
  { set_uniform(name, v.x, v.y, v.z); }

  void set_uniform(const std::string &name, const Vec4 &v) const
  { set_uniform(name, v.x, v.y, v.z, v.w); }

  // ---- matrices ------------------------------------------------------------
  // The 4x4 case every 3D program needs (u_mvp, u_view, u_projection, ...).
  // Defined in _cpp.cpp, where Mat4 is complete.
  void set_uniform(const std::string &name, const Mat4 &matrix) const;

private:
  GLuint compile_shader(GLenum type, std::string_view source);
  void check_link_errors();

  // mutable: filling the cache does not change the observable state of the
  // program, so the uniform API can stay const (like use()).
  mutable std::unordered_map<std::string, GLint> uniform_location_cache_;
};

#endif// ___CPP_H__
