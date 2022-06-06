#version 330 core

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.frag

out vec4 FragColor;
in vec2 texCoord;

uniform sampler2D texture0;

void main() {
	FragColor = texture(texture0, texCoord);
	if (FragColor.a < 0.01)
		discard;
}
