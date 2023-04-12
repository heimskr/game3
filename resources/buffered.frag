// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.frag

#version 430 core

out vec4 FragColor;
in vec2 texCoord;
flat in int index;

uniform sampler2D texture0;
uniform float divisor;
uniform int bright_tiles[8];

void main() {
	FragColor = texture(texture0, texCoord);

	if (bright_tiles[0] != index && bright_tiles[1] != index && bright_tiles[2] != index && bright_tiles[3] != index && bright_tiles[4] != index && bright_tiles[5] != index && bright_tiles[6] != index && bright_tiles[7] != index) {
		FragColor.r /= divisor;
		FragColor.g /= divisor;
		FragColor.b /= divisor;
	}
}
