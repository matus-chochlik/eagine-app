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

void main() {
	float pow2Rough = pow(vertRough, 2.0);
	float sqrtRough = sqrt(vertRough);
    vec3 ReflCoord = reflect(normalize(vertViewDir), normalize(vertNormal));
    vec3 CubeCoord = normalize(mix(ReflCoord, vertNormal, pow2Rough));

    float lod = mix(0.0, 7.0, pow2Rough);
    vec3 Reflect = mix(
        textureLod(SkyBox, CubeCoord, int(floor(lod))),
        textureLod(SkyBox, CubeCoord, int(ceil(lod))),
        fract(lod)).rgb;
    vec3 Diffuse = vec3(max(dot(vertNormal, vertLightDir)+0.1, 0.0) * 0.4);
    vec3 Ambient = mix(Reflect * mix(1.0, 0.0, vertOccl), vec3(0.2), sqrtRough);

    fragColor = vertColor * (Ambient * vertOccl + Diffuse) + Reflect * vertOccl;
}
