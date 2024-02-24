#version 330 core

layout(location = 0) in vec2  position;
layout(location = 1) in vec2  mapPosition;   // options.{x, y}
layout(location = 2) in vec2  spriteOffset;  // options.offset{X, Y}
layout(location = 3) in vec2  textureScale;  // options.scale{X, Y}
layout(location = 4) in float invertY;       // options.invertY? -1 : 1
layout(location = 5) in float spriteDegrees; // options.angle
layout(location = 6) in vec4  inSpriteColor; // options.color
layout(location = 7) in vec2  spriteSize;    // options.size{X, Y}
layout(location = 8) in vec4  inSpecialPosition;

out vec2 texCoords;
out vec4 spriteColor;
out vec4 specialPosition;

uniform mat4 projection;
uniform vec2 atlasSize; // texture_{width, height}
uniform float canvasScale;
uniform vec2 screenSize;
uniform vec2 center;
uniform float tileSize;
uniform float mapLength;

mat4 translate(mat4 matrix, vec3 delta) {
	matrix[3] = matrix[0] * delta[0] + matrix[1] * delta[1] + matrix[2] * delta[2] + matrix[3];
	return matrix;
}

mat4 scale(mat4 matrix, vec3 scale_vec) {
	matrix[0] *= scale_vec[0];
	matrix[1] *= scale_vec[1];
	matrix[2] *= scale_vec[2];
	return matrix;
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

	mat4 result = mat4(0.0);
	result[0] = matrix[0] * rotation[0][0] + matrix[1] * rotation[1][0] + matrix[2] * rotation[2][0];
	result[1] = matrix[0] * rotation[0][1] + matrix[1] * rotation[1][1] + matrix[2] * rotation[2][1];
	result[2] = matrix[0] * rotation[0][2] + matrix[1] * rotation[1][2] + matrix[2] * rotation[2][2];
	result[3] = matrix[3];
	return result;
}

void main() {
	mat4 model = mat4(1.0);
	model = scale(model, vec3(canvasScale / screenSize.x,  invertY * canvasScale / screenSize.y, 1.0)); // TODO: verify invertY correctness
	model = translate(model, vec3(-mapLength * tileSize / 2, -mapLength * tileSize / 2, 0.0));
	model = translate(model, vec3(center * tileSize, 0.0));
	model = translate(model, vec3(mapPosition.x * tileSize, mapPosition.y * tileSize, 0.0));
	model = translate(model, vec3(spriteSize / 2, 0.0));
	model = rotate(model, radians(spriteDegrees), vec3(0.0, 0.0, 1.0));
	model = translate(model, vec3(-spriteSize / 2, 0.0));
	model = translate(model, vec3(-spriteOffset.x * 2.0 * textureScale.x, -spriteOffset.y * 2.0 * textureScale.y, 0.0));
	model = scale(model, vec3(atlasSize * textureScale, 1.0));

	texCoords = position;
	spriteColor = inSpriteColor;
	specialPosition = inSpecialPosition;

	gl_Position = model * vec4(position, 0.0, 1.0);
}
