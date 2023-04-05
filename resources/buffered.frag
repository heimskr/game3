#version 330 core

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.frag

out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D texture0;
uniform sampler2D texture1;
uniform float divisor;
uniform int tile_size;
uniform int tileset_width;
uniform int bright_tiles[8];

void main() {
	FragColor = texture(texture0, texCoord);

	vec4 lightColor = texture(texture1, texCoord);


	int index_x = int((texCoord.x - 1.0 / 2048.0) * float(tileset_width / tile_size));
	int index_y = int((texCoord.y - 1.0 / 2048.0) * float(tileset_width / tile_size));
	int index = index_y * tileset_width / tile_size + index_x;
	// Silly.
	if (bright_tiles[0] != index && bright_tiles[1] != index && bright_tiles[2] != index && bright_tiles[3] != index && bright_tiles[4] != index && bright_tiles[5] != index && bright_tiles[6] != index && bright_tiles[7] != index) {
		FragColor.r /= divisor;
		FragColor.g /= divisor;
		FragColor.b /= divisor;
	}

	FragColor.r = lightColor.r;
	FragColor.b = lightColor.b;

	if (FragColor.a < 0.01)
		discard;
}
