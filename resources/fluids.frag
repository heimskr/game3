// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.frag

#version 330 core

out vec4 FragColor;
in vec2 texCoord;
flat in int index;
flat in float opacity;

uniform sampler2D texture0;
uniform float divisor;

void main() {
	FragColor = texture(texture0, texCoord);
	FragColor.r /= divisor;
	FragColor.g /= divisor;
	FragColor.b /= divisor;
	FragColor.a = opacity;
}
