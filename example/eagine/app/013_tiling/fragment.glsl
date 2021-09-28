#version 140
in vec3 vertColor;
in vec3 vertTilingCoord;
in vec2 vertTilesetCoord;
out vec3 fragColor;
uniform isampler2DArray TilingTex;
uniform sampler2DArray TilesetTex;

void main() {
	vec3 tileCoord = vec3(
		vertTilesetCoord,
		texture(TilingTex, vertTilingCoord).r);
    fragColor = vertColor * texture(TilesetTex, tileCoord).r * 2.0;
}
