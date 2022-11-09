#version 400

layout(triangles, equal_spacing, ccw) in;

uniform mat4 CameraMatrix;

in vec3 tecoPPivot[];
in vec3 tecoVPivot[];
in vec3 tecoPosition[];

out vec3 teevColor;

vec3 normalizeCoef(vec3 c) {
	return c / dot(c, vec3(1.0));
}

void main() {
	const float Eps = 0.0001;

	mat3 PPivots;
	mat3 VPivots;
	mat3 Positions;
	mat3 PivVecs;
	mat3 PosVecs;
	vec3 PivLens;
	vec3 PosLens;
	vec3 Exps;
	float AtEdge = max(gl_TessCoord.x, max(gl_TessCoord.y, gl_TessCoord.z));

	for(int c=0; c<3; ++c) {
		PPivots[c] = tecoPPivot[c];
		PivVecs[c] = tecoVPivot[c] - tecoPPivot[c];
		VPivots[c] = tecoVPivot[c];
		PosVecs[c] = tecoPosition[c] - tecoVPivot[c];
		Positions[c] = tecoPosition[c];

		PivLens[c] = length(PivVecs[c]);
		if(PivLens[c] > Eps) {
			PivVecs[c] /= PivLens[c];
		} else {
			PivVecs[c] = vec3(1.0);
		}

		PosLens[c] = length(PosVecs[c]);
		if(PosLens[c] > Eps) {
			PosVecs[c] /= PosLens[c];
		} else {
			PosVecs[c] = vec3(1.0);
		}
		Exps[c] = mix(1.0 / 3.0, 1.0, AtEdge);
	}

	vec3 Coefs = normalizeCoef(pow(gl_TessCoord.xyz, Exps));

	vec3 NewPos =
		(PPivots * Coefs) +
		normalize(PivVecs * Coefs) * dot(PivLens, Coefs) * 0.5 +
		normalize(PosVecs * Coefs) * dot(PosLens, Coefs) * 0.5 ;

    gl_Position = CameraMatrix * vec4(NewPos, 1.0);

	teevColor = vec3(0.8);
}
