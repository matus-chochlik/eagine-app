#version 140

uniform mat4 Camera;

in vec4 Position;

void main() {
    gl_Position = Camera * Position;
}
