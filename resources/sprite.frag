#version 330 core

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite.fs

in vec2 TexCoords;
out vec4 color;

uniform sampler2D sprite;
uniform vec4 spriteColor;
uniform vec4 texturePosition;

void main() {
	color = spriteColor * texture(sprite, TexCoords);
	if (color.a < 0.01)
		discard;
	else if (TexCoords.x < texturePosition.x || TexCoords.y < texturePosition.y)
		discard;
	else if (texturePosition.x + texturePosition.z < TexCoords.x)
		discard;
	else if (texturePosition.y + texturePosition.w < TexCoords.y)
		discard;
}
