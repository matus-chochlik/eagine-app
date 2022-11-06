#version 400

layout(triangles, equal_spacing, ccw) in;

uniform mat4 CameraMatrix;

in vec3 tecoPPivot[];
in vec3 tecoVPivot[];
in vec3 tecoPosition[];

out vec3 teevColor;

void main() {
	const float Eps = 0.0001;
	const float Exp = 3.0 / 2.0;
	vec3 Coef = sqrt(gl_TessCoord.xyz);
	Coef = vec3(pow(Coef.x, Exp), pow(Coef.y, Exp), pow(Coef.z, Exp));
	Coef = Coef / (Coef.x + Coef.y + Coef.z);

	mat3 PPivot;
	mat3 PivVec;
	vec3 VecLen;
	for(int r=0; r<3; ++r) {
		PPivot[r] = tecoPPivot[r];
		PivVec[r] = tecoVPivot[r] - tecoPPivot[r];
		VecLen[r] = length(PivVec[r]);
		if(VecLen[r] > Eps) {
			PivVec[r] = PivVec[r] / VecLen[r];
		} else {
			PivVec[r] = vec3(1.0);
		}
	}
	vec3 VPivot = (PPivot * Coef) + normalize(PivVec * Coef) * dot(VecLen, Coef);

	for(int r=0; r<3; ++r) {
		PivVec[r] = tecoPosition[r] - tecoVPivot[r];
		VecLen[r] = length(PivVec[r]);
		if(VecLen[r] > Eps) {
			PivVec[r] = PivVec[r] / VecLen[r];
		} else {
			PivVec[r] = vec3(1.0);
		}
	}

	vec3 Position = VPivot + normalize(PivVec * Coef) * dot(VecLen, Coef);
    gl_Position = CameraMatrix * vec4(Position, 1.0);

	teevColor = vec3(0.8);
}
