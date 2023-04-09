#version 330 core

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.vert

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 projection;
uniform vec2 map_size;

out vec2 texCoord;
out vec2 lightCoord;
out int tileIndex;

void main() {
	gl_Position = projection * vec4(aPosition, 0.0, 1.0);
	texCoord = aTexCoord;
	lightCoord = vec2(aPosition.x / map_size.x, aPosition.y / map_size.y);
}
