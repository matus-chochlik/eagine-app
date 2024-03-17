#version 140
in vec3 vertNormal;
in vec3 vertViewDir;
in vec3 vertLightDir;
in vec3 vertColor;
in float vertRough;
in float vertOccl;
out vec3 fragColor;

uniform samplerCube SkyBox;

void main() {
    vec3 ReflCoord = reflect(normalize(vertViewDir), normalize(vertNormal));
    vec3 CubeCoord = normalize(mix(ReflCoord, vertNormal, pow(vertRough, 2.0)));

    float lod = mix(0.0, 7.0, sqrt(vertRough));
    vec3 Reflect = mix(
        textureLod(SkyBox, CubeCoord, int(floor(lod))).rgb,
        textureLod(SkyBox, CubeCoord, int(ceil(lod))).rgb,
        fract(lod));
    vec3 Diffuse = vec3(max(dot(vertNormal, vertLightDir)+0.1, 0.0) * 0.4);
    vec3 Ambient = vec3(0.2) + Reflect * mix(1.0, 0.0, vertOccl);

    fragColor = vertColor * (Ambient + Diffuse) + Reflect * vertOccl;
}
