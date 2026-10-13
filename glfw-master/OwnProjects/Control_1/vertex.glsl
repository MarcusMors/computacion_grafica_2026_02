#version 430 core

// location 0 connects to: square.attribute(0, &Vertex::pos);
layout(location = 0) in vec2 pos;

uniform float u_scale;

void main(void)
{
    gl_Position = vec4(pos * u_scale, 0.0, 1.0);
}
