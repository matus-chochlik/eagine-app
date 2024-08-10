/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
#version 140
in vec4 Position;
in vec3 Normal;
in vec3 Color;
in float Occlusion;
out vec3 vertNormal;
out vec3 vertColor;
out vec3 vertLightDir;
uniform mat4 Camera;
uniform mat4 Model;
const vec3 LightDir = normalize(vec3(1.0, 1.0, 1.0));

void main() {
    gl_Position = Camera * Model * Position;
    vertNormal = mat3(Model) * Normal;
    vertColor = Color * Occlusion;
    vertLightDir = LightDir;
}
