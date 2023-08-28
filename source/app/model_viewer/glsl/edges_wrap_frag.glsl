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

	float l = mix(dot(geomNormal, LightDir), 0.45, 0.65);
	vec3 c = geomWrapCoord * 16.0;
	c = abs(c - round(c));
    vec3 FaceColor = vec3(c.x);
    vec3 EdgeColor = vec3(0.25, 0.25, 0.25);

    fragColor = mix(FaceColor * l, EdgeColor, EdgeAlpha);
}
