#version 150
in vec3 Position;
in vec2 TexCoord;
in float Occlusion;

out vec2 vertCoord;
out float vertOccl;

uniform mat4 Model, Camera;

void main() {
    gl_Position = Camera * Model * vec4(Position, 1.0);
    vertCoord = TexCoord * vec2(4.0, 2.0);
    vertOccl = Occlusion * 0.3;
}
