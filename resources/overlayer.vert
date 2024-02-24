#version 410 core

layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform dmat4 model;
uniform dmat4 projection;

void main() {
	TexCoords = vertex.zw;
	gl_Position = vec4(projection * model * vec4(vertex.xy, 0.0, 1.0));
}
