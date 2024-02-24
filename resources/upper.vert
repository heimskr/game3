// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.vert

#version 410 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 inTexCoordUpper0;
layout (location = 2) in vec2 inTexCoordUpper1;
layout (location = 3) in vec2 inTexCoordUpper2;
layout (location = 4) in vec2 inTexCoordUpper3;

uniform dmat4 projection;

out vec2 texCoordUpper0;
out vec2 texCoordUpper1;
out vec2 texCoordUpper2;
out vec2 texCoordUpper3;

void main() {
	gl_Position = vec4(projection * vec4(position, 0.0, 1.0));
	texCoordUpper0 = inTexCoordUpper0;
	texCoordUpper1 = inTexCoordUpper1;
	texCoordUpper2 = inTexCoordUpper2;
	texCoordUpper3 = inTexCoordUpper3;
}
