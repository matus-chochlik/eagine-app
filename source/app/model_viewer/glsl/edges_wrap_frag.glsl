/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
#version 140

const float EdgeWidth = 0.8;
const vec3 LightDir = normalize(vec3(1.0, 1.0, 1.0));
uniform sampler2DArray Texture0;

noperspective in vec3 geomDist;
in vec3 geomNormal;
in vec3 geomWrapCoord;
out vec3 fragColor;

void main() {
    float MinDist = min(min(geomDist.x, geomDist.y), geomDist.z);
    float EdgeAlpha = exp2(-pow(MinDist / EdgeWidth, 2.0));

    float l = mix(max(dot(geomNormal, LightDir), 1.0), 0.35, 0.55);
    vec3 c = texture(Texture0, geomWrapCoord).rgb;
    vec3 FaceColor = l * 1.0 * c;
    vec3 EdgeColor = l * 0.8 * c;

    fragColor = mix(FaceColor, EdgeColor, EdgeAlpha);
}
