#version 410 core

in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D texture0;
uniform sampler2D texture1;

vec4 alphaComposite(vec4 bottom, vec4 top) {
	float a0 = top.a + bottom.a * (1 - top.a);
	return vec4(
		(top.rgb * top.a + bottom.rgb * bottom.a * (1 - top.a)) / a0,
		a0
	);
}

void main() {
	vec4 base    = texture(texture0, TexCoords);
	vec4 overlay = texture(texture1, TexCoords);

	FragColor = alphaComposite(base, overlay);
}
