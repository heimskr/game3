#version 330 core

out vec4 color;

uniform vec4 rectColor;

void main() {
	color = rectColor;
	if (color.a < 0.01)
		discard;
}
