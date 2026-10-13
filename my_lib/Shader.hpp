#ifndef __SHADER_H__
#define __SHADER_H__

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "Shader_Source.hpp"

using namespace std;
struct P_glShaderSource
{
  // 1. shader index
  GLuint shader_id{};
  // 2. number of strings in the shader source code
  GLuint number_of_lins_in_source_code{};
  // 3. array of pointers to strings containing the shader source code
  const GLchar *shader_source_code{};
  // 4. ?
  const GLint source_code_lengths{};
  // &length -> nullptr if length is unknown and src is null-terminated
};

inline void glShaderSource_cpp(P_glShaderSource P)
{ glShaderSource(P.shader_id, P.number_of_lins_in_source_code, &P.shader_source_code, &P.source_code_lengths); }

class Shader
{
public:
  GLuint id{};

  // The constructor takes the shader type and the embedded source code
  Shader(GLenum t_type, string_view t_source) : id{ glCreateShader(t_type) }
  {
    validate_id();
    constructor(t_type, t_source);
  }

  Shader(GLenum t_type, Shader_Source t_source) : id{ glCreateShader(t_type) }
  {
    validate_id();
    constructor(t_type, t_source.view());
  }

  ~Shader() { glDeleteShader(id); }

  // Delete copy constructors to prevent accidental double-deletion of OpenGL IDs
  Shader(const Shader &) = delete;
  Shader &operator=(const Shader &) = delete;

private:
  void constructor(GLenum t_type, string_view t_source)
  {
    glShaderSource_cpp({ .shader_id = id,
      .number_of_lins_in_source_code = 1,
      .shader_source_code = t_source.data(),
      .source_code_lengths = static_cast<GLint>(t_source.size()) });
    // loads the GLSL code from the strings into the empty shader objects.

    glCompileShader(id);

    check_compile_errors();
  }

  void check_compile_errors()
  {
    GLint success{};
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (not success) {
      GLint logLength;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logLength);
      vector<char> infoLog(logLength);
      glGetShaderInfoLog(id, logLength, nullptr, infoLog.data());
      cerr << "SHADER_COMPILATION_ERROR:\n" << infoLog.data() << "\n";
    }
  }
  void validate_id() const
  {
    if (id == 0) { throw std::runtime_error("Invalid shader ID (0) – OpenGL context missing or out of memory"); }
  }
};

#endif// __SHADER_H__
