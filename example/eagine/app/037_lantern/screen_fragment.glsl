#version 140
uniform sampler2DArray Tex;

in vec2 vertLight;
in vec2 vertCoord;

out vec3 fragColor;

void main() {
	vec4 frag = texelFetch(Tex, ivec3(vertCoord, 0), 0);
	float suml = frag.a;
	vec2 ld = vertCoord - vertLight;
	int nsteps = int(max(abs(ld.x), abs(ld.y)));
	float step = 1.0 / (nsteps + 1.0);
	for(int i=0; i<=nsteps; ++i) {
		suml += texelFetch(Tex, ivec3(mix(vertLight, vertCoord, i*step), 0), 0).a;
	}

	fragColor = frag.rgb + vec3(suml*step);
}
