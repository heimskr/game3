#version 330 core

in vec2 texCoords;
in vec4 spriteColor;
in vec4 spriteComposite;
in vec4 specialPosition;
out vec4 color;

uniform sampler2D sprite;

vec4 alphaComposite(vec4 bottom, vec4 top) {
	float a0 = top.a + bottom.a * (1 - top.a);
	return vec4(
		(top.rgb * top.a + bottom.rgb * bottom.a * (1 - top.a)) / a0,
		a0
	);
}

void main() {
	vec4 pixel = texture(sprite, texCoords);
	color = alphaComposite(spriteColor * pixel, vec4(spriteComposite.rgb, min(spriteComposite.a, pixel.a)));

	if (texCoords.x < specialPosition.x || texCoords.y < specialPosition.y || specialPosition.x + specialPosition.z < texCoords.x || specialPosition.y + specialPosition.w < texCoords.y) {
		discard;
	}
}
