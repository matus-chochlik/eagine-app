#version 140
in vec3 vertNormal;
in vec3 vertViewDir;
in vec3 vertLightDir;
in vec3 vertColor;
in float vertOccl;
out vec3 fragColor;

uniform samplerCube SkyBox;

void main() {
    vec3  ReflCoord = reflect(normalize(vertViewDir), normalize(vertNormal));
    vec3 Ambient = vec3(0.4);
    vec3 Diffuse = vec3(max(dot(vertNormal, vertLightDir)+0.1, 0.0) * 0.6);
    vec3 Reflect = texture(SkyBox, ReflCoord).rgb;
    vec3 Light = mix(Reflect, Ambient, mix(0.4, 0.1, vertOccl)) + vec3(Diffuse);
    fragColor = vertColor * Light;
}
