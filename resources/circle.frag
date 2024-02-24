#version 410 core

in float centerDistance;
out vec4 color;

uniform vec4 circleColor;
uniform float cutoff;

void main() {
	color = circleColor;
	if (0 <= cutoff) {
		color.a = pow(min(1.0, 1.0 - (centerDistance * (1.0 + cutoff * 2.0) - cutoff * 2.0)), 2);
		color.rgb *= color.a;
	}
}
