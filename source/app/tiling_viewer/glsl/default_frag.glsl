#version 140
in vec3 vertNormal;
in vec3 vertTangent;
in vec3 vertBitangent;
in vec3 vertTilingCoord;
in vec2 vertTilesetCoord;
out vec3 fragColor;

uniform isampler2DArray Tiling;
uniform isampler2DArray Transition;
uniform sampler2DArray Tileset;

void main() {
    int transIndex = texture(Transition, vertTilingCoord).r;
    vec3 tileCoord = vec3(
        vertTilesetCoord,
        float(texture(Tiling, vertTilingCoord).r));
    fragColor = texture(Tileset, tileCoord).rgb * (float(transIndex) / 48.0);
}
