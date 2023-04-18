#version 430 core

layout (location = 0) in vec4 vertex;

out vec2 TexCoords;
out vec4 Position;

uniform mat4 model;
uniform mat4 projection;

void main() {
	TexCoords = vertex.zw;
	Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
	gl_Position = Position;
}
