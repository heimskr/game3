#version 330 core

in vec2 texCoords;
in vec4 spriteColor;
in vec4 specialPosition;
out vec4 color;

uniform sampler2D sprite;

void main() {
	color = spriteColor * texture(sprite, texCoords);
	if (texCoords.x < specialPosition.x || texCoords.y < specialPosition.y || specialPosition.x + specialPosition.z < texCoords.x || specialPosition.y + specialPosition.w < texCoords.y) {
		discard;
	}
}
