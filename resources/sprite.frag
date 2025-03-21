#version 330 core

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite.fs

in vec2 TexCoords;
out vec4 color;

uniform sampler2D sprite;
uniform vec4 spriteColor;
uniform vec4 spriteComposite;
uniform vec4 texturePosition;

vec4 alphaComposite(vec4 bottom, vec4 top) {
	float a0 = top.a + bottom.a * (1 - top.a);
	return vec4(
		(top.rgb * top.a + bottom.rgb * bottom.a * (1 - top.a)) / a0,
		a0
	);
}

void main() {
	vec4 pixel = texture(sprite, TexCoords);
	color = alphaComposite(spriteColor * pixel, vec4(spriteComposite.rgb, min(spriteComposite.a, pixel.a)));

	if (TexCoords.x < texturePosition.x || TexCoords.y < texturePosition.y || texturePosition.x + texturePosition.z < TexCoords.x || texturePosition.y + texturePosition.w < TexCoords.y) {
		discard;
	}
}
