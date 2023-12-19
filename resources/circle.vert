#version 330 core

layout (location = 0) in vec2 vertex;

out float centerDistance;

uniform mat4 model;
uniform mat4 projection;

void main() {
	gl_Position = projection * model * vec4(vertex, 0.0, 1.0);
	centerDistance = distance(vertex.xy, vec2(0, 0));
}
