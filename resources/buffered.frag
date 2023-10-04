// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.frag

#version 330 core

out vec4 FragColor;
in vec2 texCoordBase0;
in vec2 texCoordBase1;
in vec2 texCoordBase2;
in vec2 texCoordBase3;
in vec2 texCoordUpper0;
in vec2 texCoordUpper1;
in vec2 texCoordUpper2;
in vec2 texCoordUpper3;
in vec2 fluidTexCoord;
flat in float fluidOpacity;
// flat in int index;

uniform sampler2D texture0;
// uniform float divisor;
// uniform int bright_tiles[8];

vec4 alphaComposite(vec4 bottom, vec4 top) {
	float a0 = top.a + bottom.a * (1 - top.a);
	return vec4(
		(top.r * top.a + bottom.r * bottom.a * (1 - top.a)) / a0,
		(top.g * top.a + bottom.g * bottom.a * (1 - top.a)) / a0,
		(top.b * top.a + bottom.b * bottom.a * (1 - top.a)) / a0,
		a0
	);
}

void main() {
	vec4 base0  = texture(texture0, texCoordBase0);
	vec4 base1  = texture(texture0, texCoordBase1);
	vec4 fluid  = texture(texture0, fluidTexCoord);
	vec4 base2  = texture(texture0, texCoordBase2);
	vec4 base3  = texture(texture0, texCoordBase3);
	vec4 upper0 = texture(texture0, texCoordUpper0);
	vec4 upper1 = texture(texture0, texCoordUpper1);
	vec4 upper2 = texture(texture0, texCoordUpper2);
	vec4 upper3 = texture(texture0, texCoordUpper3);

	fluid.a *= fluidOpacity;

	vec4 mix1 = alphaComposite(base0, base1);
	vec4 mix2 = alphaComposite(mix1,  fluid);
	vec4 mix3 = alphaComposite(mix2,  base2);
	vec4 mix4 = alphaComposite(mix3,  base3);
	vec4 mix5 = alphaComposite(mix4,  upper0);
	vec4 mix6 = alphaComposite(mix5,  upper1);
	vec4 mix7 = alphaComposite(mix6,  upper2);

	FragColor = alphaComposite(mix7,  upper3);

	// FragColor.a = 1.0;
	// FragColor.r = 1.0;

	// if (bright_tiles[0] != index && bright_tiles[1] != index && bright_tiles[2] != index && bright_tiles[3] != index && bright_tiles[4] != index && bright_tiles[5] != index && bright_tiles[6] != index && bright_tiles[7] != index) {
	// 	FragColor.r /= divisor;
	// 	FragColor.g /= divisor;
	// 	FragColor.b /= divisor;
	// }
}
