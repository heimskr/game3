#version 330 core

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite.vs

layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;
// uniform vec2 offset;

void main() {
	// TexCoords = vertex.zw + offset;
	TexCoords = vertex.zw;
	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}
