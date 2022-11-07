#version 400

layout(vertices = 3) out;

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

	gl_TessLevelOuter[gl_InvocationID] =
		min(1 + int((64.0 * vertEdgeLen[gl_InvocationID]) / EdgeDist), 120);

    if(gl_InvocationID == 0) {
		gl_TessLevelInner[0] =
			min(1 + int((64.0 * vertFaceArea[0]) / EdgeDist), 120);
    }
}
