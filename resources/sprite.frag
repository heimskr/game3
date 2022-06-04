#version 330 core

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite.fs

in vec2 TexCoords;
out vec4 color;

uniform sampler2D sprite;
uniform vec3 spriteColor;

void main() {
	color = vec4(spriteColor, 1.0) * texture(sprite, TexCoords);
}
