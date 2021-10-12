#version 330
in vec3 vertNormal;
in vec2 vertWrapCoord;

out vec3 fragColor;

uniform float LightMult;
uniform vec3 LightDir;
uniform sampler2D ColorTex;
uniform sampler2D LightTex;

void main() {
    vec3 color = texture(ColorTex, vertWrapCoord).rgb;
    float occl = texture(LightTex, vertWrapCoord).r;

    float ambient = occl;
    float diffuse = max(dot(normalize(LightDir), vertNormal), 0.0)*
        pow(occl, 4.0);

    fragColor = color * ((ambient + diffuse) * LightMult);
}
