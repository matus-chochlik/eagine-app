#version 400

uniform vec3 CameraPosition;

in vec3 Position;
in vec3 PPivot;
in vec3 VPivot;
in float EdgeLen;
in float FaceArea;

out vec3 vertPPivot;
out vec3 vertVPivot;
out vec3 vertPosition;
out float vertEdgeLen;
out float vertFaceArea;
out float vertDistance;

void main() {
	vertPPivot = PPivot;
	vertVPivot = VPivot;
    vertPosition = Position;
	vertEdgeLen = EdgeLen;
	vertFaceArea = FaceArea;
    vertDistance = length(CameraPosition - Position);
}
