// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite.vs

#version 330 core

layout (location = 0) in vec2  position;
// layout (location = 1) in vec2  inTexCoords;
layout (location = 1) in vec2  mapPosition;   // options.{x, y}
layout (location = 2) in vec2  textureOffset; // options.offset{X, Y}
layout (location = 3) in vec2  textureScale;  // options.scale{X, Y}
layout (location = 4) in float invertY;       // options.invertY? -1 : 1
layout (location = 5) in float spriteDegrees; // options.angle
layout (location = 6) in vec4  inSpriteColor; // options.color
layout (location = 7) in vec4  inSpecialPosition;

out vec2 texCoords;
out vec4 spriteColor;
out vec4 specialPosition;

uniform mat4 projection;
uniform vec2 atlasSize; // texture_{width, height}

mat4 translate(mat4 matrix, vec3 delta) {

	mat4 transp = transpose(matrix);
	transp[3] = transp[0] * delta[0] + transp[1] * delta[1] + transp[2] * delta[2] + transp[3];
	return transpose(transp);

	// vec4 row0 = vec4(matrix[0][0], matrix[1][0], matrix[2][0], matrix[3][0]) * delta[0];
	// vec4 row1 = vec4(matrix[0][1], matrix[1][1], matrix[2][1], matrix[3][1]) * delta[1];
	// vec4 row2 = vec4(matrix[0][2], matrix[1][2], matrix[2][2], matrix[3][2]) * delta[2];
	// vec4 row3 = vec4(matrix[0][3], matrix[1][3], matrix[2][3], matrix[3][3]);
	// matrix[0][3] = row0[0] + row1[0] + row2[0] + row3[0];
	// matrix[1][3] = row0[1] + row1[1] + row2[1] + row3[1];
	// matrix[2][3] = row0[2] + row1[2] + row2[2] + row3[2];
	// matrix[3][3] = row0[3] + row1[3] + row2[3] + row3[3];
	// return matrix;
}

mat4 scale(mat4 matrix, vec3 scale_vec) {
	mat4 transp = transpose(matrix);
	transp[0] *= scale_vec[0];
	transp[1] *= scale_vec[1];
	transp[2] *= scale_vec[2];
	return transpose(transp);
}

vec3 normalize3(vec3 vec) {
	return vec * inversesqrt(dot(vec, vec));
}

mat4 rotate(mat4 matrix, float angle, vec3 axis) {
	float c = cos(angle);
	float s = sin(angle);
	axis = normalize(axis);
	vec3 temp = (1.0 - c) * axis;

	mat4 rotation = mat4(0.0);
	rotation[0][0] = c + temp[0] * axis[0];
	rotation[1][0] = temp[0] * axis[1] + s * axis[2];
	rotation[2][0] = temp[0] * axis[2] - s * axis[1];
	rotation[0][1] = temp[1] * axis[0] - s * axis[2];
	rotation[1][1] = c + temp[1] * axis[1];
	rotation[2][1] = temp[1] * axis[2] + s * axis[0];
	rotation[0][2] = temp[2] * axis[0] + s * axis[1];
	rotation[1][2] = temp[2] * axis[1] - s * axis[0];
	rotation[2][2] = c + temp[2] * axis[2];

	mat4 transp = transpose(matrix);

	mat4 result = mat4(0.0);
	result[0] = transp[0] * rotation[0][0] + transp[1] * rotation[1][0] + transp[2] * rotation[2][0];
	result[1] = transp[0] * rotation[0][1] + transp[1] * rotation[1][1] + transp[2] * rotation[2][1];
	result[2] = transp[0] * rotation[0][2] + transp[1] * rotation[1][2] + transp[2] * rotation[2][2];
	result[3] = transp[3];
	return transpose(result);
}

void main() {
	mat4 model = mat4(1.0);
	model = translate(model, vec3(mapPosition.x * 16.0 - textureOffset.x * 2.0 * textureScale.x,
	                              mapPosition.y * 16.0 - textureOffset.y * 2.0 * textureScale.y,
	                              0.0));
	model = scale(model, vec3(1.0, invertY, 1.0));
	// model = translate(model, vec3(0.5 * atlasSize.x, 0.5 * atlasSize.y, 0.0));
	// model = rotate(model, radians(spriteDegrees), vec3(0.0, 0.0, 1.0));
	// model = translate(model, vec3(-0.5 * atlasSize.x, -0.5 * atlasSize.y, 0.0));
	model = scale(model, vec3(atlasSize.x * textureScale.x, atlasSize.y * textureScale.y, 1.0));
	// texCoords = inTexCoords;
	texCoords = position;
	spriteColor = inSpriteColor;
	specialPosition = inSpecialPosition;
	gl_Position = projection * model * vec4(position, 0.0, 1.0);
}
