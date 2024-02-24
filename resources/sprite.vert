// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite.vs

#version 410 core

layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>
out vec2 TexCoords;

uniform dmat4 model;
uniform dmat4 projection;

void main() {
	TexCoords = vertex.zw;
	gl_Position = vec4(projection * model * vec4(vertex.xy, 0.0, 1.0));
}
