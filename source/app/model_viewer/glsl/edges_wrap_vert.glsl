/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
#version 140

uniform mat4 Camera;
uniform mat4 Model;

in vec4 Position;
in vec3 Normal;
in vec3 WrapCoord;
out vec3 vertNormal;
out vec3 vertWrapCoord;

void main() {
    gl_Position = Camera * Model * Position;
    vertNormal = mat3(Model) * Normal;
    vertWrapCoord = WrapCoord;
}
