#version 330 core

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/GeometryRenderer.frag

uniform sampler2D texture0;
in vec2 texCoord;
out vec4 FragColor;

void main() {
	FragColor = texture(texture0, texCoord);
}
