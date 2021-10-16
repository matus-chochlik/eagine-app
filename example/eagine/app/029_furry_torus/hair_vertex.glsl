#version 150
in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;
in float Occlusion;

out vec3 vertColor;
out vec3 vertNormal;
out float vertOccl;
out float vertLength;

uniform mat4 Model;
uniform sampler2D Tex;

void main() {
    gl_Position = Model * vec4(Position, 1.0);
    vertColor = texture(Tex, TexCoord * vec2(4.0, 2.0)).rgb;
	vertNormal = mat3(Model) * Normal;
    vertOccl = Occlusion;
	vertLength = Occlusion;
}
