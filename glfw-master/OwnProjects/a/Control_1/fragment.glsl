#version 430 core

in vec3 v_color;// interpolated vertex color from the vertex shader

// Set from C++ with: program.set_uniform("u_pulse", pulse);
uniform float u_pulse;

out vec4 color;

void main(void)
{
  color = vec4(v_color * u_pulse, 1.0);
}
