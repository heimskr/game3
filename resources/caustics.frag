#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D txr;
uniform sampler2D pathmap;

void main() {
	float value = texture(pathmap, TexCoords).r * 255.0;
	color = vec4(0.5, 0.5, 1.0, texture(txr, TexCoords * 4).r * 0.5 * value);
}
