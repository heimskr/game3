#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D top;
uniform sampler2D txr;

vec4 invert(vec4 color) {
	return vec4(1 - color.rgb, color.a);
}

void main() {
	color = texture(txr, TexCoords) / invert(texture(top, TexCoords));
}
