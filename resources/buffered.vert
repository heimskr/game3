// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.vert

#version 330 core

layout (location =  0) in vec2 position;
layout (location =  1) in vec2 inTexCoordBase0;
layout (location =  2) in vec2 inTexCoordBase1;
layout (location =  3) in vec2 inTexCoordBase2;
layout (location =  4) in vec2 inTexCoordBase3;
layout (location =  5) in vec2 inTexCoordUpper0;
layout (location =  6) in vec2 inTexCoordUpper1;
layout (location =  7) in vec2 inTexCoordUpper2;
layout (location =  8) in vec2 inTexCoordUpper3;
layout (location =  9) in vec2 inFluidTexCoord;
layout (location = 10) in float inFluidOpacity;

uniform mat4 projection;

out vec2 texCoordBase0;
out vec2 texCoordBase1;
out vec2 texCoordBase2;
out vec2 texCoordBase3;
out vec2 texCoordUpper0;
out vec2 texCoordUpper1;
out vec2 texCoordUpper2;
out vec2 texCoordUpper3;
out vec2 fluidTexCoord;
flat out float fluidOpacity;

void main() {
	gl_Position = projection * vec4(position, 0.0, 1.0);
	texCoordBase0  = inTexCoordBase0;
	texCoordBase1  = inTexCoordBase1;
	texCoordBase2  = inTexCoordBase2;
	texCoordBase3  = inTexCoordBase3;
	texCoordUpper0 = inTexCoordUpper0;
	texCoordUpper1 = inTexCoordUpper1;
	texCoordUpper2 = inTexCoordUpper2;
	texCoordUpper3 = inTexCoordUpper3;
	fluidTexCoord  = inFluidTexCoord;
	fluidOpacity   = inFluidOpacity;
}
