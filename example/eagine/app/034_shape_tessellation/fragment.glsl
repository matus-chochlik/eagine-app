#version 400

in vec3 geomColor;
noperspective in vec3 geomEdge;

out vec3 fragColor;

void main() {
    float EdgeDist = min(min(geomEdge.x, geomEdge.y), geomEdge.z);
    float EdgeAlpha = exp2(-pow(EdgeDist, 2.0));

    vec3 FaceColor = geomColor;
    const vec3 EdgeColor = vec3(0.2, 0.2, 0.2);

    fragColor = mix(FaceColor, EdgeColor, EdgeAlpha);
}
