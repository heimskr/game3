#version 410 core

flat in dvec2 texCoords;
flat in dvec4 spriteColor;
flat in dvec4 specialPosition;
out vec4 color;

uniform sampler2D sprite;

void main() {
	color = vec4(spriteColor * texture(sprite, vec2(texCoords)));
	if (texCoords.x < specialPosition.x || texCoords.y < specialPosition.y || specialPosition.x + specialPosition.z < texCoords.x || specialPosition.y + specialPosition.w < texCoords.y) {
		discard;
	}
}
