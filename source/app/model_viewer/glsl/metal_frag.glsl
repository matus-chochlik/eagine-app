#version 140
in vec3 vertNormal;
in vec3 vertViewDir;
in vec3 vertLightDir;
in vec3 vertColor;
in float vertOccl;
out vec3 fragColor;

uniform samplerCube SkyBox;

void main() {
    vec3 ReflCoord = reflect(normalize(vertViewDir), normalize(vertNormal));
    vec3 CubeCoord = normalize(mix(vertNormal, ReflCoord, vertOccl));

    float lod = mix(7.0, 0.0, pow(vertOccl, 2.0));
    vec3 Reflect = mix(
        textureLod(SkyBox, CubeCoord, int(floor(lod))).rgb,
        textureLod(SkyBox, CubeCoord, int(ceil(lod))).rgb,
        fract(lod));
    vec3 Diffuse = vec3(max(dot(vertNormal, vertLightDir)+0.1, 0.0) * 0.6);
    vec3 Ambient = vec3(0.2) + Reflect * mix(1.0, 0.0, vertOccl);

    fragColor = vertColor * (Ambient + Diffuse) + Reflect * vertOccl;
}
