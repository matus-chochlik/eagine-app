/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
#version 140
in vec3 vertNormal;
in vec3 vertViewDir;
in vec3 vertLightDir;
in vec3 vertColor;
in float vertRough;
in float vertOccl;
out vec3 fragColor;

uniform samplerCube SkyBox;
const float maxLod = 7.0;

void main() {
	float pow2Rough = pow(vertRough, 2.0);
	float sqrtRough = sqrt(vertRough);
    vec3 ReflCoord = reflect(normalize(vertViewDir), normalize(vertNormal));
    vec3 CubeCoord = normalize(mix(ReflCoord, vertNormal, pow2Rough));

    float rlod = mix(0.0, maxLod, pow2Rough);
    vec4 Reflect = mix(
        textureLod(SkyBox, CubeCoord, int(floor(rlod))),
        textureLod(SkyBox, CubeCoord, int(ceil(rlod))),
        fract(rlod));
    float alod = mix(maxLod-1.0, maxLod, sqrtRough);
    vec4 Ambient = mix(
        textureLod(SkyBox, CubeCoord, int(floor(alod))),
        textureLod(SkyBox, vertNormal, int(ceil(alod))),
        fract(alod));

    vec3 Diffuse = vec3(max(dot(vertNormal, vertLightDir)+0.1, 0.0) * 0.4);

	fragColor =
		(Ambient.rgb * min(Ambient.a + 0.2, 1.0) + Diffuse) * vertOccl * vertColor +
		(Reflect.rgb * min(Reflect.a + 0.1, 1.0)) * vertOccl;

}
