/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
#version 140
in vec4 Position;
in vec3 Normal;
in vec3 Color;
in float Roughness;
in float Occlusion;
out vec3 vertNormal;
out vec3 vertViewDir;
out vec3 vertLightDir;
out vec3 vertColor;
out float vertRough;
out float vertOccl;

uniform mat4 Camera;
uniform mat4 Model;
const vec3 LightDir = normalize(vec3(1.0, 1.0, 1.0));

void main() {
    gl_Position = Camera * Model * Position;
    vec3 cameraLoc = (vec4(0.0, 0.0, 0.0, 1.0) * Camera).xyz;
    vertNormal = mat3(Model) * Normal;
    vertViewDir = cameraLoc - Position.xyz;
    vertLightDir = LightDir;
    vertColor = Color * Occlusion;
    vertRough = Roughness;
    vertOccl = Occlusion;
}
