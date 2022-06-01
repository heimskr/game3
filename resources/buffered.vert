#version 330 core

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.vert

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;
uniform mat4 projection;

out vec2 texCoord;

void main() {
	gl_Position = projection * vec4(aPosition, 0.0, 1.0);
	texCoord = aTexCoord;
}
