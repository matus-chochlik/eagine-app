/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
#version 140

const float EdgeWidth = 0.8;

noperspective in vec3 geomDist;
in vec3 geomNormal;
out vec3 fragColor;

void main() {
    float MinDist = min(min(geomDist.x, geomDist.y), geomDist.z);
    float EdgeAlpha = exp2(-pow(MinDist / EdgeWidth, 2.0));

    vec3 FaceColor = normalize(vec3(1) - geomNormal);
    vec3 EdgeColor = vec3(0.0, 0.0, 0.0);

    fragColor = mix(FaceColor, EdgeColor, EdgeAlpha);
}
