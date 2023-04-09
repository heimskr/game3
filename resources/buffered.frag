#version 430 core

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.frag

out vec4 FragColor;
in vec2 texCoord;
in vec2 lightCoord;
flat in int index;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float divisor;
uniform int tile_size;
uniform int tileset_width;
uniform int bright_tiles[8];

void main() {
	FragColor = texture(texture0, texCoord);
	vec4 lightColor = texture(texture1, lightCoord);
	if (bright_tiles[0] != index && bright_tiles[1] != index && bright_tiles[2] != index && bright_tiles[3] != index && bright_tiles[4] != index && bright_tiles[5] != index && bright_tiles[6] != index && bright_tiles[7] != index) {
		FragColor.r /= divisor;
		FragColor.g /= divisor;
		FragColor.b /= divisor;
	}

	if (FragColor.a < 0.01) {
		discard;
	} else if (0.01 <= lightColor.a) {
		FragColor = FragColor * (2.0 + lightColor.a * divisor) * lightColor;
		FragColor.a = 1.0;
	}
}
