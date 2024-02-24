// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/BufferedRenderer.vert

#version 410 core

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 inTexCoordBase0;
layout (location = 2) in vec2 inTexCoordBase1;
layout (location = 3) in vec2 inTexCoordBase2;
layout (location = 4) in vec2 inTexCoordBase3;
layout (location = 5) in vec2 inFluidTexCoord;
layout (location = 6) in float inFluidOpacity;

uniform dmat4 projection;

out vec2 texCoordBase0;
out vec2 texCoordBase1;
out vec2 texCoordBase2;
out vec2 texCoordBase3;
out vec2 fluidTexCoord;
flat out float fluidOpacity;

void main() {
	gl_Position = vec4(projection * vec4(position, 0.0, 1.0));
	texCoordBase0 = inTexCoordBase0;
	texCoordBase1 = inTexCoordBase1;
	texCoordBase2 = inTexCoordBase2;
	texCoordBase3 = inTexCoordBase3;
	fluidTexCoord = inFluidTexCoord;
	fluidOpacity  = inFluidOpacity;
}
