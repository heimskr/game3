#version 330 core

precision mediump float;

in vec2 TexCoords;
in vec4 Position;
out vec4 FragColor;
uniform sampler2D texture0;
uniform sampler2D texture1;

void main() {
	vec4 base    = texture(texture0, TexCoords);
	vec4 overlay = texture(texture1, TexCoords);

	float max_alpha = max(base.a, overlay.a);

	if (0.01 <= overlay.a) {
		FragColor = base * 2.0 * overlay;
		FragColor.a = max_alpha;
	} else {
		FragColor = base;
	}

	// FragColor = Position;
}
