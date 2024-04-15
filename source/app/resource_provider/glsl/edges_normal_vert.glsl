/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
#version 140

uniform mat4 Camera;

in vec4 Position;
in vec3 Normal;
out vec3 vertNormal;

void main() {
    gl_Position = Camera * Position;
    vertNormal = Normal;
}
