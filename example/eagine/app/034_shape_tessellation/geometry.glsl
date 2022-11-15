#version 400

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

uniform vec2 ViewportDimensions;

in vec3 teevColor[];

out vec3 geomColor;
noperspective out vec3 geomEdge;

void main() {
    vec2 ScreenPos[3];
    for(int i = 0; i != 3; ++i) {
        ScreenPos[i] =
          ViewportDimensions * gl_in[i].gl_Position.xy / gl_in[i].gl_Position.w;
    }

    vec2 TmpVect[3];
    for(int i = 0; i != 3; ++i) {
        TmpVect[i] = ScreenPos[(i + 2) % 3] - ScreenPos[(i + 1) % 3];
    }

    const vec3 EdgeMask[3] = vec3[3](
		vec3(1.0, 0.0, 0.0),
		vec3(0.0, 1.0, 0.0),
		vec3(0.0, 0.0, 1.0));

    for(int i = 0; i != 3; ++i) {
        float Dist = abs(
                       TmpVect[(i + 1) % 3].x * TmpVect[(i + 2) % 3].y -
                       TmpVect[(i + 1) % 3].y * TmpVect[(i + 2) % 3].x) /
                     length(TmpVect[i]);
        vec3 DistVect = vec3(Dist, Dist, Dist);

        gl_Position = gl_in[i].gl_Position;
        geomColor = teevColor[i];
        geomEdge = EdgeMask[i] * DistVect;
        EmitVertex();
    }
    EndPrimitive();
}
