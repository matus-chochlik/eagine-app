#version 140
in vec3 Position;
in vec3 Normal;
in vec3 TexCoord;
out vec3 vertColor;
out vec3 vertTilingCoord;
out vec2 vertTilesetCoord;
uniform mat4 Camera;

void main() {
    gl_Position = Camera * vec4(Position, 1.0);
    vertColor = mix(normalize(vec3(1.0) + Normal), vec3(1.0), 0.4);
    vertTilingCoord = TexCoord;
	vertTilesetCoord = 256.0 * TexCoord.xy;
}
