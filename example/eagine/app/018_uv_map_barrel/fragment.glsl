#version 140
in vec3 vertNormal;
in vec2 vertWrapCoord;

out vec3 fragColor;

uniform sampler2D ColorTex;
uniform sampler2D LightTex;
uniform sampler2D RoughTex;

const vec3 lightDir = normalize(vec3(7.0, 8.0, 9.0));

void main() {
    vec3 color = texture(ColorTex, vertWrapCoord).rgb;
    float aoccl = texture(LightTex, vertWrapCoord).r;
    float rough = texture(RoughTex, vertWrapCoord).r;
	float ldot = dot(lightDir, vertNormal);

    float ambient = aoccl;
    float diffuse = max(ldot, 0.0) * mix(0.5, 1.0, aoccl);
	float specular = pow(max(ldot+0.02, 0.0), mix(8.0, 2.0, rough));

    fragColor = color * (ambient + diffuse) + vec3(specular * 0.4);
}
