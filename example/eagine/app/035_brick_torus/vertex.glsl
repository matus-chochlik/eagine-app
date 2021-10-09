#version 400

uniform mat4 Model;

in vec4 Position;
in vec3 Normal;
in vec3 Tangent;
in vec2 TexCoord;

out vec3 vertNormal;
out vec3 vertTangent;
out vec2 vertTexCoord;

void main() {
	gl_Position = Model * Position;
	vertNormal = mat3(Model) * Normal;
	vertTangent = mat3(Model) * Tangent;
	vertTexCoord = vec2(4.0*TexCoord.x, 2.0*TexCoord.y+TexCoord.x);
}
