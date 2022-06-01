#version 330 core

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/GeometryRenderer.vert

uniform mat4 projection;
uniform ivec2 mapSize; // Number of tiles in the tilemap, not the tileset
uniform ivec2 setSize; // Number of tiles in the tileset, not the tilemap

layout (location = 0) in uint aTileId;

out VS_OUT {
	uint tileId;
} vs_out;

void main() {
	int i = gl_VertexID;
	// float x = float(i / mapSize.y);
	// float y = float(i % mapSize.y);
	float x = float(i % mapSize.x);
	float y = float(i / mapSize.x);
	gl_Position = vec4(x, y, 0, 1);
	vs_out.tileId = aTileId;
}
