#version 140
in vec3 Position;
in vec3 Normal;
in vec3 Color;
out vec3 vertNormal;
out vec3 vertColor;
out vec3 vertLightDir;
uniform mat4 Camera;
const vec3 LightDir = vec3(1.0, 1.0, 1.0);

void main() {
    gl_Position = Camera * vec4(Position, 1.0);
	vertNormal = Normal;
    vertColor = Color;
	vertLightDir = LightDir;
}