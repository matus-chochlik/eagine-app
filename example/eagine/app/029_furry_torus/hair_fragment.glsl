#version 150
in vec3 geomColor;
in float geomOccl;
out vec3 fragColor;

void main() {
    fragColor = geomColor * geomOccl;
}
