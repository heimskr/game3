#version 410 core

layout (location = 0) in vec4 vertex;

uniform dmat4 model;
uniform dmat4 projection;

void main() {
	gl_Position = vec4(projection * model * vec4(vertex.xy, 0.0, 1.0));
}
