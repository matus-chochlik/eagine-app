#version 400

layout(triangles, equal_spacing, ccw) in;

uniform mat4 CameraMatrix;

in vec3 tecoPosition[];
in vec3 tecoVPivot[];

out vec3 teevColor;

void main() {
	mat3 org;
	mat3 vec;
	vec3 len;
	for(int r=0; r<3; ++r) {
		org[r] = tecoVPivot[r];
		vec[r] = tecoPosition[r] - tecoVPivot[r];
		len[r] = length(vec[r]);
		vec[r] = vec[r] / len[r];
	}

	vec3 c = sqrt(gl_TessCoord.xyz);
	float e = 3.0 / 2.0;
	c = vec3(pow(c.x, e), pow(c.y, e), pow(c.z, e));
	float q = c.x+c.y+c.z;
	c /= q;
	vec3 o = org * c;
	vec3 v = normalize(vec * c);
	float l = dot(len, c);

    gl_Position = CameraMatrix * vec4(o + v * l, 1.0);
	teevColor = vec3(0.8);
}
