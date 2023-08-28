#version 140

const float EdgeWidth = 0.8;

noperspective in vec3 geomDist;
in vec3 geomNormal;
in vec3 geomWrapCoord;
out vec3 fragColor;

const vec3 LightDir = vec3(1.0, 1.0, 1.0);

void main() {
    float MinDist = min(min(geomDist.x, geomDist.y), geomDist.z);
    float EdgeAlpha = exp2(-pow(MinDist / EdgeWidth, 2.0));

	float l = mix(max(dot(geomNormal, LightDir), 1.0), 0.35, 0.55);
	vec3 w = geomWrapCoord * 32.0;
	w = abs(w - round(w));
	w = exp(-w * 256.0);
	w = min(w, 1.0);
	float c = 1.0 - max(w.x, w.y);
    vec3 FaceColor = vec3(l * c * 1.0);
    vec3 EdgeColor = vec3(l * c * 0.8);

    fragColor = mix(FaceColor, EdgeColor, EdgeAlpha);
}
