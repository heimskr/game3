#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D txr;
uniform sampler2D pathmap;
uniform vec4 texturePosition;

void main() {
	vec2 coords = TexCoords;
	vec2 intpart;
	float value = texture(pathmap, modf(coords * 8, intpart)).r * 255.0;
	color = vec4(0.5, 0.5, 1.0, texture(txr, coords).r * 0.5 * value);

	if (coords.x < texturePosition.x || coords.y < texturePosition.y || texturePosition.x + texturePosition.z < coords.x || texturePosition.y + texturePosition.w < coords.y) {
		discard;
	}
}
