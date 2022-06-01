#version 330 core

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/GeometryRenderer.geom

uniform mat4 projection;
uniform ivec2 mapSize;

in VS_OUT {
	uint tileId;
} gs_in[];

out vec2 texCoord;

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

void main() {
	float mSx = float(mapSize.x);
	float mSy = float(mapSize.y);

	uint tileId = gs_in[0].tileId & 255u;
	float tileX = float(tileId % uint(mapSize.x)) / mSx;
	float tileY = float(tileId / uint(mapSize.x)) / mSy;

	const float B = 1 / 256.0;
	float Sx = 1 / mSx;
	float Sy = 1 / mSy;

	gl_Position = projection * gl_in[0].gl_Position;
	texCoord = vec2(tileX + B, tileY + B);
	EmitVertex();

	gl_Position = projection * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
	texCoord = vec2(tileX + Sx - B, tileY + B);
	EmitVertex();

	gl_Position = projection * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
	texCoord = vec2(tileX + B, tileY + Sy - B);
	EmitVertex();

	gl_Position = projection * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0));
	texCoord = vec2(tileX + Sx - B, tileY + Sy - B);
	EmitVertex();

	EndPrimitive();
}
