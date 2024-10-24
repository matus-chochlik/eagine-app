#version 140
uniform mat4 Camera;
uniform mat4 Model;

in vec3 Position;
in vec3 Normal;

out vec3 vertNormal;

void main() {
    gl_Position = Camera * Model * vec4(Position, 1.0);
	vertNormal = Normal;
}
