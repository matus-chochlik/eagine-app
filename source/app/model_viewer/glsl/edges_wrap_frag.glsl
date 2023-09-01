#version 140

const float EdgeWidth = 0.8;
const vec3 LightDir = normalize(vec3(1.0, 1.0, 1.0));
uniform sampler2DArray Checker;

noperspective in vec3 geomDist;
in vec3 geomNormal;
in vec3 geomWrapCoord;
out vec3 fragColor;

void main() {
    float MinDist = min(min(geomDist.x, geomDist.y), geomDist.z);
    float EdgeAlpha = exp2(-pow(MinDist / EdgeWidth, 2.0));

    float l = mix(max(dot(geomNormal, LightDir), 1.0), 0.35, 0.55);
    float c = texture(Checker, vec3(geomWrapCoord.xy, 0.0)).r;
    vec3 FaceColor = vec3(l * c * 1.0);
    vec3 EdgeColor = vec3(l * c * 0.8);

    fragColor = mix(FaceColor, EdgeColor, EdgeAlpha);
}
