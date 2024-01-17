#version 140
in vec3 vertNormal;
in vec3 vertTangent;
in vec3 vertBitangent;
in vec3 vertWrapCoord;
out vec3 fragColor;

void main() {
    fragColor = vertNormal;
}
