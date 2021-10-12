#version 140
in vec3 Position;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;
in vec2 WrapCoord;

out mat3 vertNormalMat;
out vec2 vertWrapCoord;

uniform mat4 Camera;

void main() {
    gl_Position = Camera * vec4(Position, 1.0);
	vertNormalMat = mat3(Tangent, Bitangent, Normal);
    vertWrapCoord = WrapCoord;
}
