#version 140
uniform sampler2DArray Tex;
uniform float CandleLight;
uniform float AmbientLight;

in vec3 vertNormal;
in vec3 vertLightDir;
in vec2 vertWrapCoord;

out vec4 fragColor;

void main() {
    vec3 color = texture(Tex, vec3(vertWrapCoord, 0.0)).rgb;
    vec3 light = texture(Tex, vec3(vertWrapCoord, 1.0)).rgb;

	float ldot = dot(vertLightDir, vertNormal);

	float ambient = light.r;
	float diffuse = (light.g + light.b) * max(ldot, 0.0) * CandleLight;
	float scatter = light.g * max(-ldot, 0.0) * pow(CandleLight, 2.0);

    fragColor = vec4(
		color * (ambient * AmbientLight + diffuse * 1.75 + scatter * 0.35),
		light.b * CandleLight);
}
