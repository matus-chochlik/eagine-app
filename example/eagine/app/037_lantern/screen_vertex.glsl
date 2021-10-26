#version 140
uniform vec2 ScreenSize;

in vec3 Position;
in vec2 Coord;

out vec2 vertLight;
out vec2 vertCoord;

void main() {
    gl_Position = vec4(Position, 1.0);
	vertLight = 0.5 * ScreenSize;
	vertCoord = Coord * ScreenSize;
}
