// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.vert

#version 330 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 inTexCoordUpper0;
layout (location = 2) in vec2 inTexCoordUpper1;
layout (location = 3) in vec2 inTexCoordUpper2;
layout (location = 4) in vec2 inTexCoordUpper3;
layout (location = 5) in vec2 inTexCoordUpper4;
layout (location = 6) in vec2 inTexCoordUpper5;
layout (location = 7) in vec2 inTexCoordUpper6;
layout (location = 8) in vec2 inTexCoordUpper7;

uniform mat4 projection;

out vec2 texCoordUpper0;
out vec2 texCoordUpper1;
out vec2 texCoordUpper2;
out vec2 texCoordUpper3;
out vec2 texCoordUpper4;
out vec2 texCoordUpper5;
out vec2 texCoordUpper6;
out vec2 texCoordUpper7;

void main() {
	gl_Position = projection * vec4(position, 0.0, 1.0);
	texCoordUpper0 = inTexCoordUpper0;
	texCoordUpper1 = inTexCoordUpper1;
	texCoordUpper2 = inTexCoordUpper2;
	texCoordUpper3 = inTexCoordUpper3;
	texCoordUpper4 = inTexCoordUpper4;
	texCoordUpper5 = inTexCoordUpper5;
	texCoordUpper6 = inTexCoordUpper6;
	texCoordUpper7 = inTexCoordUpper7;
}
