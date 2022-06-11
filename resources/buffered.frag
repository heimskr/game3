#version 330 core

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.frag

out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D texture0;
uniform float time;

void main() {
	FragColor = texture(texture0, texCoord);
	float mult = (1 + sin(time*5)) + 1;
	FragColor.r /= mult;
	FragColor.g /= mult;
	FragColor.b /= mult;
	if (FragColor.a < 0.01)
		discard;
}
