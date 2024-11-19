#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D txr;
uniform sampler2D pathmap;
uniform float time;

void main() {
	vec2 coords = TexCoords + vec2(sin(TexCoords.y * 8 + time) / 32, 0);
	float value = texture(pathmap, coords).r * 255.0;
	color = vec4(0.5, 0.5, 1.0, texture(txr, coords * 4).r * 0.5);
}
