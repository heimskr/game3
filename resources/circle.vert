#version 410 core

layout (location = 0) in vec2 vertex;

out float centerDistance;

uniform dmat4 model;
uniform dmat4 projection;

void main() {
	gl_Position = vec4(projection * model * vec4(vertex, 0.0, 1.0));
	centerDistance = distance(vertex.xy, vec2(0, 0));
}
