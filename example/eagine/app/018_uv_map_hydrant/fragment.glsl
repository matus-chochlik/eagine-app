#version 140
in mat3 vertNormalMat;
in vec2 vertWrapCoord;

out vec3 fragColor;

uniform sampler2DArray Tex;
uniform vec3 LightDir;

void main() {
    vec3 color = texture(Tex, vec3(vertWrapCoord, 0.0)).rgb;
    vec3 normal = texture(Tex, vec3(vertWrapCoord, 1.0)).rgb;
    vec3 light = texture(Tex, vec3(vertWrapCoord, 2.0)).rgb;

	normal = normalize(vec3(normal.x*2.0 - 1.0, normal.y*2.0 - 1.0, normal.z));
	normal = vertNormalMat * normal;
	float ldot = dot(LightDir, normal);

	float ambient = light.r;
	float diffuse = light.g * max(ldot, 0.0);
	float specular = pow(max(ldot+0.01, 0.0), mix(2.0, 8.0, light.b))*light.b;

    fragColor = color * (ambient * 0.25 + diffuse * 0.85) + vec3(specular);
}
