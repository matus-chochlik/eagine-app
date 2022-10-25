#version 400

uniform vec3 LightPosition;
uniform sampler2DArray TextureMap;

flat in mat3 geomPositionFront;
flat in mat3 geomNormalFront;
flat in mat3 geomTangentFront;
flat in mat3 geomTexCoordFront;
flat in vec3 geomWFront;

noperspective in vec3 geomBarycentric;

in vec3 geomPosition;
in vec3 geomNormal;
in vec3 geomTangent;
in vec3 geomTexCoord;

out vec3 fragColor;

vec3 vcdiv(vec3 a, vec3 b) {
	return vec3(a.x/b.x, a.y/b.y, a.z/b.z);
}

void main() {
	const vec3 one = vec3(1.0);

	vec3 bzfv = vcdiv(geomBarycentric,geomWFront);
	float idobzfv = 1.0/dot(one,bzfv);

	vec3 p0 = geomPosition;
	vec3 p1 = (geomPositionFront*bzfv)*idobzfv;

	vec3 n0 = geomNormal;
	vec3 n1 = (geomNormalFront*bzfv)*idobzfv;

	vec3 t0 = geomTangent;
	vec3 t1 = (geomTangentFront*bzfv)*idobzfv;

	vec3 tc0 = geomTexCoord;
	vec3 tc1 = (geomTexCoordFront*bzfv)*idobzfv;

	float tl = textureQueryLod(TextureMap, tc1.xy).x;
	ivec2 ts = textureSize(TextureMap, int(tl)).xy;
	int mts = max(ts.x, ts.y);
	vec2 dtc = tc1.xy - tc0.xy;
	float mdtc = max(abs(dtc.x), abs(dtc.y));

	int nsam = max(min(int(mdtc*mts), mts), 1);
	float step = 1.0 / nsam;
	for(int s=0; s<=nsam; ++s) {
		vec3 tc = mix(tc1, tc0, s*step);
		float height = texture(TextureMap, vec3(tc.xy, 1.0)).a;
		if(tc.z <= height+0.01) {
			vec3 p = mix(p1, p0, s*step);
			vec3 n = mix(n1, n0, s*step);
			vec3 t = mix(t1, t0, s*step);
			vec3 b = cross(n, t);

			vec3 clr = texture(TextureMap, vec3(tc.xy, 0.0)).rgb;
			vec3 nml = texture(TextureMap, vec3(tc.xy, 1.0)).rgb;
			vec3 ldir = normalize(LightPosition - p);
			nml = vec3(2.0*(nml.x-0.5), 2.0*(nml.y-0.5), nml.z);
			nml = normalize(t*nml.x+b*nml.y+n*nml.z);
			float ldot = max(dot(ldir, nml)+0.1, 0.0);
			float ambi = mix(0.05, 0.55, height);
			float diff = 1.4 * sqrt(ldot);
			float lght = mix(ambi, mix(ambi, diff, max(sqrt(height), ldot)), 0.45);
			fragColor = clr * lght;
			return;
		}
	}
	discard;
}
