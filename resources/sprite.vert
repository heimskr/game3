// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite.vs

#version 330 core

layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>
out vec2 TexCoords;
out vec4 position;
out vec4 originalVertex;

uniform mat4 model;
uniform mat4 projection;

void main() {
	originalVertex = vertex;
	TexCoords = vertex.zw;

	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
	position = gl_Position;
}
