#version 150
in vec2 vertCoord;
in float vertOccl;
out vec3 fragColor;

uniform sampler2D Tex;

void main() {
    fragColor = texture(Tex, vertCoord).rgb * mix(0.6, 0.2, vertOccl);
}
