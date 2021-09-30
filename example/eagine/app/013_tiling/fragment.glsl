#version 140
in vec3 vertColor;
in vec3 vertTilingCoord;
in vec2 vertTilesetCoord;
in vec2 vertGridCoord;
out vec3 fragColor;
uniform isampler2DArray TilingTex;
uniform sampler2DArray TilesetTex;

void main() {
	vec3 tileCoord = vec3(
		vertTilesetCoord,
		texture(TilingTex, vertTilingCoord).r);
	vec2 gridCoord = abs(vertGridCoord - round(vertGridCoord));
	float grid = 2.0 - exp(-min(gridCoord.x, gridCoord.y)*64.0) * 0.5;
    fragColor = vertColor * texture(TilesetTex, tileCoord).r * grid;
}
