#version 330

uniform mat4 Camera;

in vec4 Position;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 WrapCoord;
out vec3 vertNormal;
out vec3 vertTangent;
out vec3 vertBitangent;
out vec3 vertTilingCoord;
out vec2 vertTilesetCoord;

const vec2 TilingRes = vec2(1024.0); // TODO: uniform

void main() {
    gl_Position = Camera * Position;
    vertNormal = Normal;
    vertTangent = Tangent;
    vertBitangent = Bitangent;
    vertTilingCoord = WrapCoord;
	vertTilesetCoord = TilingRes * WrapCoord.xy;
}
