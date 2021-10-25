#version 140
in vec3 vertNormal;
in vec3 vertLightDir;
in vec2 vertWrapCoord;

out vec3 fragColor;

uniform sampler2DArray Tex;
uniform float LightPower;

void main() {
    vec3 color = texture(Tex, vec3(vertWrapCoord, 0.0)).rgb;
    vec3 light = texture(Tex, vec3(vertWrapCoord, 1.0)).rgb;

	float ldot = dot(vertLightDir, vertNormal);

	float ambient = light.r;
	float diffuse = (light.g + light.b) * max(ldot, 0.0) * LightPower;
	float scatter = light.g * max(-ldot, 0.0) * pow(LightPower, 2.0);

    fragColor = color * (ambient * 0.05 + diffuse * 1.75 + scatter * 0.35);
}
