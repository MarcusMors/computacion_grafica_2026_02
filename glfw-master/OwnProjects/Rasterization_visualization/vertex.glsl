#version 430 core

layout(location = 0) in vec2 aPos;   // world-space position (grid units)
layout(location = 1) in vec3 aColor; // per-vertex color

uniform mat4 uMVP;                   // orthographic projection

out vec3 vColor;

void main(void)
{
    vColor = aColor;
    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
}
