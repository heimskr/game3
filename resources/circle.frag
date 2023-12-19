#version 330 core

in float radiusDistance;
out vec4 color;

uniform vec4 circleColor;
uniform vec2 radius;
uniform float cutoff;

void main() {
	color = circleColor;
	if (0 <= cutoff) {
		color.a = min(1.0, 1.0 - (radiusDistance * (1.0 + cutoff * 2.0) - cutoff * 2.0));
	}
}
