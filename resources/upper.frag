// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.frag

#version 330 core

out vec4 FragColor;
in vec2 texCoordUpper0;
in vec2 texCoordUpper1;
in vec2 texCoordUpper2;
in vec2 texCoordUpper3;
in vec2 texCoordUpper4;
in vec2 texCoordUpper5;
in vec2 texCoordUpper6;
in vec2 texCoordUpper7;

uniform sampler2D texture0;

vec4 alphaComposite(vec4 bottom, vec4 top) {
	float a0 = top.a + bottom.a * (1 - top.a);
	return vec4(
		(top.rgb * top.a + bottom.rgb * bottom.a * (1 - top.a)) / a0,
		a0
	);
}

void main() {
	vec4 upper0 = texture(texture0, texCoordUpper0);
	vec4 upper1 = texture(texture0, texCoordUpper1);
	vec4 upper2 = texture(texture0, texCoordUpper2);
	vec4 upper3 = texture(texture0, texCoordUpper3);
	vec4 upper4 = texture(texture0, texCoordUpper4);
	vec4 upper5 = texture(texture0, texCoordUpper5);
	vec4 upper6 = texture(texture0, texCoordUpper6);
	vec4 upper7 = texture(texture0, texCoordUpper7);

	if (upper0.a == 0.0) {
		if (upper1.a == 0.0 && upper2.a == 0.0 && upper3.a == 0.0 && upper4.a == 0.0 && upper5.a == 0.0 && upper6.a == 0.0 && upper7.a == 0.0) {
			discard;
		} else {
			upper0 = vec4(0.2, 0.2, 0.2, 1.0);
		}
	}

	vec4 mix1 = alphaComposite(upper0, upper1);
	vec4 mix2 = alphaComposite(mix1,   upper2);
	vec4 mix3 = alphaComposite(mix2,   upper3);
	vec4 mix4 = alphaComposite(mix3,   upper4);
	vec4 mix5 = alphaComposite(mix4,   upper5);
	vec4 mix6 = alphaComposite(mix5,   upper6);
	vec4 mix7 = alphaComposite(mix6,   upper7);

	FragColor = mix7;
}
