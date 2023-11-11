#version 140

uniform mat4 Camera;

in vec4 Position;
in vec3 Normal;
out vec3 vertNormal;

void main() {
    gl_Position = Camera * Position;
    vertNormal = Normal;
}
