#version 330

in vec4 Position;
uniform vec3 LightDir;

void main() {
	gl_Position = Position;
}

