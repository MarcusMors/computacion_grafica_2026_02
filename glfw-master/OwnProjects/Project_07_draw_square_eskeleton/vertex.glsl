#version 430

uniform float offset;

void main(void) {
    switch (gl_VertexID) {
        case 0:
            gl_Position = vec4(0.25, 0.25, 0, 1);
            break;
        case 1:
            gl_Position = vec4(0.25, -0.25, 0,1);
            break;
        case 2:
            gl_Position = vec4(-0.25, -0.25, 0,1);
            break;
        case 3:
            gl_Position = vec4(-0.25, 0.25, 0,1);
            break;
        case 4:
            gl_Position = vec4(0.25, 0.25, 0, 1);
            break;
    }
}
