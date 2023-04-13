#version 430 core

in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D texture0;
uniform sampler2D texture1;

void main() {
	vec4 base    = texture(texture0, TexCoords);
	vec4 overlay = texture(texture1, TexCoords);

	float max_alpha = max(base.a, overlay.a);

	if (0.01 <= overlay.a) {
		FragColor = base * overlay;
		FragColor.a = max_alpha;
	} else {
		FragColor = base;
	}
}
