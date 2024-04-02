/// Copyright Matus Chochlik.
/// Distributed under the Boost Software License, Version 1.0.
/// See accompanying file LICENSE_1_0.txt or copy at
/// https://www.boost.org/LICENSE_1_0.txt
///
#version 140
in vec2 Position;
out vec2 vertCoord;
void main() {
    gl_Position = vec4(Position, 0.0, 1.0);
    vertCoord = Position;
};
