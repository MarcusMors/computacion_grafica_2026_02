#version 430 core

// Outline color, set from C++ with: program.set_uniform("u_color", color);
uniform vec3 u_color;

out vec4 color;

void main(void)
{
  color = vec4(u_color, 1.0);
}
