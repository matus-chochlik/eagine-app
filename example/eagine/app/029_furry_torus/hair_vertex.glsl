#version 150
in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;
in float Occlusion;

out vec3 vertColor;
out vec3 vertNormal;
out float vertOccl;
out float vertLength;

uniform mat4 CurrModel, PrevModel;
uniform sampler2D Tex;

void main() {
    gl_Position = CurrModel * vec4(Position, 1.0);
    vertColor = texture(Tex, TexCoord).rgb;
	vertNormal = mat3(PrevModel) * Normal;
    vertOccl = Occlusion;
	vertLength = pow(Occlusion * 1.1, 2.0);
}
