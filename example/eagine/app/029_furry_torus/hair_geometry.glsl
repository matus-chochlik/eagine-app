#version 150
layout(points) in;
layout(line_strip, max_vertices = 12) out;

in vec3 vertColor[1];
in vec3 vertNormal[1];
in float vertOccl[1];
in float vertLength[1];

out vec3 geomColor;
out float geomOccl;

uniform mat4 Camera;

void main() {
	float len = vertLength[0];
	float str = abs(dot(vertNormal[0], vec3(0.0, 1.0, 0.0)));

	vec3 nml[2];
	nml[0] = normalize(vertNormal[0] + vec3(0.0, -0.1, 0.0));
	nml[1] = normalize(nml[0] + vec3(0.0, -mix(2.4, 0.8, str), 0.0));

	vec3 pos[3];
	pos[0] = gl_in[0].gl_Position.xyz;
	pos[1] = pos[0]+nml[0]*len*0.04;
	pos[2] = pos[1]+nml[1]*len*0.06;

	float occ[3];
	occ[0] = mix(0.6, 0.2, vertOccl[0]);
	occ[1] = mix(0.8, 0.5, vertOccl[0]);
	occ[2] = 1.0;

	geomColor = vertColor[0];

	for(int i=0; i<12; ++i) {
		float t = i / 11.0;
		float a = pow(t, 2.0);
		float b = 2.0*(1.0-t)*t;
		float c = pow(1.0-t, 2.0);

		gl_Position = Camera * vec4(a*pos[0] + b*pos[1] + c*pos[2], 1.0);
		geomOccl = a*occ[0] + b*occ[1] + c*occ[2];
		EmitVertex();
	}

	EndPrimitive();
}

