#version 140
#extension GL_ARB_shading_language_include : enable

#include </checker>
#include </mandelbrot>

out vec3 fragColor;

void main() {
    vec2 co = gl_TexCoord[0].xy;
    float ck = checker(co);
    float mb = mandelbrot(co);
    fragColor = mix(vec3(mb), gl_Color.rgb, ck);
}
