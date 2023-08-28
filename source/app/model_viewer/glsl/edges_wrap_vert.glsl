#version 140

uniform mat4 Camera;

in vec4 Position;
in vec3 Normal;
in vec3 WrapCoord;
out vec3 vertNormal;
out vec3 vertWrapCoord;

void main() {
    gl_Position = Camera * Position;
    vertNormal = Normal;
    vertWrapCoord = WrapCoord;
}
