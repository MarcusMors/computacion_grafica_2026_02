#version 430 core

// location 0 connects to: square.attribute(0, &Vertex::pos);
layout(location = 0) in vec3 pos;

uniform mat3 uTransform;
void main() {
    vec3 p = uTransform * pos;      // translation lives in the matrix
    gl_Position = vec4(p.xy, 0.0, 1.0);
}