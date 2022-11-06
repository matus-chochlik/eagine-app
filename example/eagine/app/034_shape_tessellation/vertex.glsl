#version 400

uniform vec3 CameraPosition;

in vec3 Position;
in vec3 VPivot;

out vec3 vertPosition;
out vec3 vertVPivot;
out float vertDistance;

void main() {
    vertPosition = Position;
	vertVPivot = VPivot;
    vertDistance = length(CameraPosition - Position);
}
