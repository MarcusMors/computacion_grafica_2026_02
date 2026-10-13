#version 430 core

// location 0 connects to: pyramid.attribute(0, &Vertex::pos);
// location 1 connects to: pyramid.attribute(1, &Vertex::color);
layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_color;

// Set from C++ with: program.set_uniform("u_mvp", mvp);
uniform mat4 u_mvp;

out vec3 v_color;// interpolated across the triangle by the rasterizer

void main(void)
{
    v_color = a_color;
    gl_Position = u_mvp * vec4(a_pos, 1.0);
}
