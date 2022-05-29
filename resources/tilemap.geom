#version 330 core

// Credit: https://github.com/davudk/OpenGL-TileMap-Demos/blob/master/Resources/GeometryRenderer.geom

uniform mat4 projection;

in VS_OUT {
	uint tileId;
} gs_in[];

out vec2 texCoord;

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

void main() {

	const uint T = 32u;
	const float F = float(T);

	uint tileId = gs_in[0].tileId & 255u;
	float tileX = float(tileId & (T - 1)) / F;
	float tileY = float((tileId >> log2(T)) & (T - 1)) / F;

	const float B = 1 / 256.0;
	const float S = 1 / F;

	gl_Position = projection * gl_in[0].gl_Position;
	texCoord = vec2(tileX + B, tileY + B);
	EmitVertex();

	gl_Position = projection * (gl_in[0].gl_Position + vec4(1.0, 0.0, 0.0, 0.0));
	texCoord = vec2(tileX + S - B, tileY + B);
	EmitVertex();

	gl_Position = projection * (gl_in[0].gl_Position + vec4(0.0, 1.0, 0.0, 0.0));
	texCoord = vec2(tileX + B, tileY + S - B);
	EmitVertex();

	gl_Position = projection * (gl_in[0].gl_Position + vec4(1.0, 1.0, 0.0, 0.0));
	texCoord = vec2(tileX + S - B, tileY + S - B);
	EmitVertex();

	EndPrimitive();
}
