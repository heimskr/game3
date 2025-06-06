// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.frag

#version 330 core

out vec4 FragColor;
in vec2 texCoordBase0;
in vec2 texCoordBase1;
in vec2 texCoordBase2;
in vec2 texCoordBase3;
in vec2 texCoordBase4;
in vec2 texCoordBase5;
in vec2 texCoordBase6;
in vec2 texCoordBase7;
in vec2 fluidTexCoord;
flat in float fluidOpacity;

uniform sampler2D texture0;

vec4 alphaComposite(vec4 bottom, vec4 top) {
	float a0 = top.a + bottom.a * (1 - top.a);
	return vec4(
		(top.rgb * top.a + bottom.rgb * bottom.a * (1 - top.a)) / a0,
		a0
	);
}

void main() {
	vec4 base0 = texture(texture0, texCoordBase0);
	vec4 base1 = texture(texture0, texCoordBase1);
	vec4 base2 = texture(texture0, texCoordBase2);
	vec4 base3 = texture(texture0, texCoordBase3);
	vec4 base4 = texture(texture0, texCoordBase4);
	vec4 base5 = texture(texture0, texCoordBase5);
	vec4 fluid = texture(texture0, fluidTexCoord);
	vec4 base6 = texture(texture0, texCoordBase6);
	vec4 base7 = texture(texture0, texCoordBase7);

	if (base0.a == 0.0) {
		base0 = vec4(0.2, 0.2, 0.2, 1.0);
	}

	fluid.a *= fluidOpacity;

	vec4 mix1 = alphaComposite(base0, base1);
	vec4 mix2 = alphaComposite(mix1,  base2);
	vec4 mix3 = alphaComposite(mix2,  base3);
	vec4 mix4 = alphaComposite(mix3,  base4);
	vec4 mix5 = alphaComposite(mix4,  base5);
	vec4 mix6 = alphaComposite(mix5,  fluid);
	vec4 mix7 = alphaComposite(mix6,  base6);
	vec4 mix8 = alphaComposite(mix7,  base7);

	FragColor = mix8;
}
