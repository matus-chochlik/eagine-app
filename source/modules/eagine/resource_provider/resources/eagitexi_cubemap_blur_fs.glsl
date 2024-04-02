/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
#version 140
in vec2 vertCoord;
out vec4 fragColor;
uniform samplerCube cubeMap;
uniform int faceIdx;
uniform int cubeSide;
uniform int sharpness;

mat3 getCubeFace(int f) {
    return mat3[6](
      mat3(0.0, 0.0, -1.0, 0.0, -1.0, 0.0, 1.0, 0.0, 0.0),
      mat3(0.0, 0.0, 1.0, 0.0, -1.0, 0.0, -1.0, 0.0, 0.0),
      mat3(1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0),
      mat3(1.0, 0.0, 0.0, 0.0, 0.0, -1.0, 0.0, -1.0, 0.0),
      mat3(1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 1.0),
      mat3(-1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0))[f];
}

vec3 getCubeCoord() {
    mat3 cubeFace = getCubeFace(faceIdx);
    return cubeFace[0] * vertCoord.x + cubeFace[1] * vertCoord.y + cubeFace[2];
}

vec3 getSampleCoord(int f, int x, int y) {
    float ics = 1.0 / float(cubeSide - 1);
    mat3 face = getCubeFace(f);
    vec2 sampleCoord =
      vec2(mix(-1.0, 1.0, float(x) * ics), mix(-1.0, 1.0, float(y) * ics));
    return face[0] * sampleCoord.x + face[1] * sampleCoord.y + face[2];
}

void main() {
    vec3 cubeCoord = getCubeCoord();
    vec4 accumColor = vec4(0.0);
    float accumWeight = 0.0;
    for(int f = 0; f < 6; ++f) {
        for(int y = 0; y < cubeSide; ++y) {
            for(int x = 0; x < cubeSide; ++x) {
                vec3 sampleCubeCoord = getSampleCoord(f, x, y);
                vec4 sampleColor = texture(cubeMap, sampleCubeCoord);
                float sampleWeight = pow(
                  max(
                    dot(normalize(cubeCoord), normalize(sampleCubeCoord)), 0.0),
                  pow(2.0, float(sharpness)));
                accumColor = accumColor + sampleColor * sampleWeight;
                accumWeight += sampleWeight;
            }
        }
    }
    fragColor = accumColor / accumWeight;
}

