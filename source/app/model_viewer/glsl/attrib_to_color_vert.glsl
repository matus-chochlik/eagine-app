#version 140
in vec4 Position;
in vec3 Attrib;
out vec3 vertColor;
uniform mat4 Camera;

void main() {
    gl_Position = Camera * Position;
    vertColor = Attrib;
}
