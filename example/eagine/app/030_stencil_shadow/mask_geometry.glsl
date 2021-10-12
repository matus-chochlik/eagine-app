#version 330
layout(triangles_adjacency) in;
layout(triangle_strip, max_vertices = 12) out;

uniform mat4 Camera;
uniform vec3 LightDir;

vec3 position(int i) {
    return gl_in[i].gl_Position.xyz;
}

vec3 triangleNormal(int a, int b, int c) {
    return normalize(
      cross(position(b) - position(c), position(a) - position(c)));
}

void makeVertex(int i, float u, float v, float s) {
    float n = v * s;
    gl_Position = Camera * vec4(position(i) - LightDir  * n, 1.0);
    EmitVertex();
}

void processEdge(int a, int b, int c, int o) {
    vec3 ni = triangleNormal(a, b, c);
    vec3 no = triangleNormal(b, a, o);

    float nid = dot(ni, LightDir);
    float nod = dot(no, LightDir);

    if((nid >= 0.0) && (nod <= 0.0)) {
        vec3 n = normalize(mix(ni, no, 0.5));
        float s = 10.0;
        makeVertex(a, 0.0, 0.0, s);
        makeVertex(a, 0.0, 1.0, s);
        makeVertex(b, 1.0, 0.0, s);
        makeVertex(b, 1.0, 1.0, s);
        EndPrimitive();
    }
}

void main() {
    processEdge(0, 2, 4, 1);
    processEdge(2, 4, 0, 3);
    processEdge(4, 0, 2, 5);
}
