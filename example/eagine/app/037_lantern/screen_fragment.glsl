#version 140
uniform sampler2DRect Tex;

in vec2 vertCoord;

out vec3 fragColor;

void main() {
    //fragColor = texture(Tex, vertCoord).rgb;
    fragColor = vec3(texture(Tex, vertCoord).a);
}
