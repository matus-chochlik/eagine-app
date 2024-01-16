#version 140
in vec4 Position;
in vec3 Normal;
in vec3 Color;
in float Occlusion;
out vec3 vertNormal;
out vec3 vertViewDir;
out vec3 vertLightDir;
out vec3 vertColor;
out float vertOccl;

uniform mat4 Camera;
const vec3 LightDir = normalize(vec3(1.0, 1.0, 1.0));

void main() {
    gl_Position = Camera * Position;
    vec3 cameraLoc = (vec4(0.0, 0.0, 0.0, 1.0) * Camera).xyz;
    vertNormal = Normal;
    vertViewDir = cameraLoc - Position.xyz;
    vertLightDir = LightDir;
    vertColor = Color * Occlusion;
    vertOccl = Occlusion;
}
