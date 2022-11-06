#version 400

uniform vec3 CameraPosition;

in vec3 Position;
in vec3 PPivot;
in vec3 VPivot;

out vec3 vertPPivot;
out vec3 vertVPivot;
out vec3 vertPosition;
out float vertDistance;

void main() {
	vertPPivot = PPivot;
	vertVPivot = VPivot;
    vertPosition = Position;
    vertDistance = length(CameraPosition - Position);
}
