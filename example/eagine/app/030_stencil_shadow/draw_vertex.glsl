#version 330
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 WrapCoord;

out vec3 vertNormal;
out vec2 vertWrapCoord;

uniform mat4 Camera;

void main() {
    gl_Position = Camera * vec4(Position, 1.0);
    vertNormal = Normal;
    vertWrapCoord = WrapCoord;
}
