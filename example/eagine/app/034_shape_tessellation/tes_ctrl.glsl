#version 400

layout(vertices = 3) out;

uniform float Factor = 1.0;

in vec3 vertPPivot[];
in vec3 vertVPivot[];
in vec3 vertPosition[];
in float vertEdgeLen[];
in float vertFaceArea[];
in float vertDistance[];

out vec3 tecoPPivot[];
out vec3 tecoVPivot[];
out vec3 tecoPosition[];

void main() {
	vec3 Dist = vec3(vertDistance[0], vertDistance[1], vertDistance[2]);
	vec3 Mask = vec3(1.0);
	Mask[gl_InvocationID] = 0.0;

    tecoPPivot[gl_InvocationID] = vertPPivot[gl_InvocationID];
    tecoVPivot[gl_InvocationID] = vertVPivot[gl_InvocationID];
    tecoPosition[gl_InvocationID] = vertPosition[gl_InvocationID];

	float EdgeDist = dot(Dist, Mask * 0.5);

	float len = vertEdgeLen[gl_InvocationID];
	gl_TessLevelOuter[gl_InvocationID] =
		min(1 + int((Factor * 64.0 * len) / EdgeDist), 120);

    if(gl_InvocationID == 0) {
		const float Eps = 0.0001;
		mat3 Vecs;
		for(int i = 0; i < 3; ++i) {
			vec3 v1 = tecoVPivot[i] - tecoPPivot[i];
			vec3 v2 = tecoPosition[i] - tecoVPivot[i];
			float l1 = length(v1);
			float l2 = length(v2);
			if(l1 > Eps) {
				v1 /= l1;
			} else {
				v1 = vec3(0.0);
			}
			if(l2 > Eps) {
				v2 /= l2;
			} else {
				v2 = vec3(0.0);
			}
			if(l1 + l2 > Eps) {
				Vecs[i] = normalize(mix(v1, v2, 0.5));
			} else {
				Vecs[i] = vec3(0.0);
			}
		}
		vec3 Dots;
		for(int i = 0; i < 3; ++i) {
			Dots[i] = dot(Vecs[(i + 1) % 3], Vecs[(i + 2) % 3]);
		}
		
		float Flatness = max(0.0, dot(Dots, vec3(1.0 / 3.0)));


		len = mix(0.5, 0.1, Flatness) * (
			min(vertEdgeLen[0], min(vertEdgeLen[1], vertEdgeLen[2]))+
			sqrt(vertFaceArea[0]));
		gl_TessLevelInner[0] =
			min(1 + int((Factor * 64.0 * len) / EdgeDist), 120);
    }
}
