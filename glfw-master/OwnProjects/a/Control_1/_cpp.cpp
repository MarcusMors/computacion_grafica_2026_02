#include "_cpp.hpp"
#include "Mat4.hpp"// complete type for set_uniform(name, Mat4)
#include <cstdint>
#include <stdexcept>

using namespace std;

void glDrawArrays_cpp(P_glDrawArrays P) { glDrawArrays(static_cast<GLenum>(P.mode), P.start_index, P.count); }

static GLsizei index_type_size(GLenum type)
{
  switch (type) {
    case GL_UNSIGNED_BYTE: return 1;
    case GL_UNSIGNED_SHORT: return 2;
    case GL_UNSIGNED_INT: return 4;
    default: throw std::invalid_argument(
        "glDrawElements_cpp: type must be GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT or GL_UNSIGNED_INT");
  }
}

void glDrawElements_cpp(P_glDrawElements P)
{
  // first_index is in INDICES; glDrawElements wants a byte offset disguised as const void*
  glDrawElements(static_cast<GLenum>(P.mode), P.count, P.type,
                 reinterpret_cast<const void *>(static_cast<std::uintptr_t>(P.first_index) * index_type_size(P.type)));
}


void request_opengl_version()
{
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}
GLFWwindow *glfwCreateWindow(P_glfwCreateWindow window_settings)
{
  auto ws = window_settings;
  GLFWwindow *window = glfwCreateWindow(ws.width, ws.height, ws.title.c_str(),
                                        ws.fullscreen_mode, ws.resource_sharing);
  return window;
}

void Program::validate_id()
{
  if (id == 0) { throw std::runtime_error("Failed to create OpenGL program object (no context?)"); }
}


void Program::attach(const Shader_Pipeline &pipeline)
{
  // Validate tessellation pair
  const bool has_tcs = !pipeline.tessellation_control.empty();
  const bool has_tes = !pipeline.tessellation_evaluation.empty();
  if (has_tcs != has_tes) {
    throw std::runtime_error("Tessellation control and evaluation must both be present or both absent");
  }

  // Compile each stage that has source
  vector<GLuint> shaders;

  auto compile = [&](GLenum type, string_view source, string name) {
    if (!source.empty()) {
      GLuint shader = compile_shader(type, source);
      glAttachShader(id, shader);
      shaders.push_back(shader);
    }
  };

  compile(GL_VERTEX_SHADER, pipeline.vertex.view(), "vertex");
  compile(GL_TESS_CONTROL_SHADER, pipeline.tessellation_control.view(), "tessellation control");
  compile(GL_TESS_EVALUATION_SHADER, pipeline.tessellation_evaluation.view(), "tessellation evaluation");
  compile(GL_GEOMETRY_SHADER, pipeline.geometry.view(), "geometry");
  compile(GL_FRAGMENT_SHADER, pipeline.fragment.view(), "fragment");

  // Link the program
  glLinkProgram(id);
  check_link_errors();

  // Shader objects are no longer needed after linking
  for (GLuint shader : shaders) glDeleteShader(shader);
}

GLuint Program::compile_shader(GLenum type, string_view source)
{
  GLuint shader = glCreateShader(type);
  const char *src = source.data();
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (not success) {
    char log[512];
    glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
    throw std::runtime_error("Shader compilation failed: " + string{ log });
  }
  return shader;
}

void Program::check_link_errors()
{
  GLint success;
  glGetProgramiv(id, GL_LINK_STATUS, &success);
  if (!success) {
    GLchar infoLog[512];
    glGetProgramInfoLog(id, sizeof(infoLog), nullptr, infoLog);
    cerr << "ERROR: Program linking failed:\n" << infoLog << endl;
  }
}

// ----------------------------------------------------------------------------
// Uniform overload that needs the complete Mat4 type (Mat4.hpp).
// GL_FALSE = do not transpose: Mat4 is already stored column-major.
// ----------------------------------------------------------------------------
void Program::set_uniform(const std::string &name, const Mat4 &matrix) const
{
  use();
  glUniformMatrix4fv(uniform_location(name), 1, GL_FALSE, matrix.data());
}
