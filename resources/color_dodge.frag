#version 330 core

in vec2 TexCoords;
out vec4 color;

uniform sampler2D top;
uniform sampler2D txr;

vec4 invert(vec4 color) {
	return vec4(1 - color.rgb, color.a);
}

vec4 dodge(vec4 bottom, vec4 top) {
	return bottom / invert(top);
}

void main() {
	color = dodge(texture(txr, TexCoords), texture(top, TexCoords));
}
