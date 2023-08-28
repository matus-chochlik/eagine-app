#version 140
in vec3 vertNormal;
in vec3 vertColor;
in vec3 vertLightDir;
out vec3 fragColor;

void main() {
    float Ambient = 0.4;
    float Diffuse = max(dot(vertNormal, vertLightDir)+0.1, 0.0) * 0.6;
    float Light = (Ambient + Diffuse);
    fragColor = vertColor * Light;
}
