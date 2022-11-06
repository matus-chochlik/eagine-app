#version 400

layout(vertices = 3) out;

in vec3 vertPosition[];
in vec3 vertVPivot[];
in float vertDistance[];

out vec3 tecoPosition[];
out vec3 tecoVPivot[];

int tessLevel(float dist) {
    return 1 + int(256.0 / pow(dist * 0.5 + 1.0, 2.0));
}

void main() {
    tecoPosition[gl_InvocationID] = vertPosition[gl_InvocationID];
    tecoVPivot[gl_InvocationID] = vertVPivot[gl_InvocationID];

    if(gl_InvocationID == 0) {
        gl_TessLevelInner[0] = tessLevel(
          (vertDistance[0] + vertDistance[1] + vertDistance[2]) * 0.33333);
        gl_TessLevelOuter[0] =
          tessLevel((vertDistance[1] + vertDistance[2]) * 0.5);
        gl_TessLevelOuter[1] =
          tessLevel((vertDistance[2] + vertDistance[0]) * 0.5);
        gl_TessLevelOuter[2] =
          tessLevel((vertDistance[0] + vertDistance[1]) * 0.5);
    }
}
