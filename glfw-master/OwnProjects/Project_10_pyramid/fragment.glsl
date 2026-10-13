#version 430 core

in vec3 v_color;// plain per-face color coming from the vertex shader

out vec4 color;

void main(void)
{
  color = vec4(v_color, 1.0);
}
