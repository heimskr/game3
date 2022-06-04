#version 330 core

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite.fs

in vec2 TexCoords;
out vec4 color;

uniform sampler2D sprite;
uniform vec4 spriteColor;

void main() {
	color = spriteColor * texture(sprite, TexCoords);
	if (color.a < 0.01)
		discard;
}
