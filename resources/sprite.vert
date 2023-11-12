// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite.vs

#version 330 core

layout(location = 0) in vec2  position;
layout(location = 1) in vec2  inTexCoords;
layout(location = 2) in vec2  mapPosition;   // options.{x, y}
layout(location = 3) in vec2  textureOffset; // options.offset{X, Y}
layout(location = 4) in vec2  textureScale;  // options.scale{X, Y}
layout(location = 5) in float invertY;       // options.invertY? -1 : 1
layout(location = 6) in float spriteDegrees; // options.angle
layout(location = 7) in vec4  inSpriteColor; // options.color
layout(location = 8) in vec2  spriteSize;    // options.size{X, Y}
layout(location = 9) in vec4  inSpecialPosition;

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
	mat4 transp = (matrix);
	transp[0] *= scale_vec[0];
	transp[1] *= scale_vec[1];
	transp[2] *= scale_vec[2];
	return (transp);
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

	mat4 transp = (matrix);

	mat4 result = mat4(0.0);
	result[0] = transp[0] * rotation[0][0] + transp[1] * rotation[1][0] + transp[2] * rotation[2][0];
	result[1] = transp[0] * rotation[0][1] + transp[1] * rotation[1][1] + transp[2] * rotation[2][1];
	result[2] = transp[0] * rotation[0][2] + transp[1] * rotation[1][2] + transp[2] * rotation[2][2];
	result[3] = transp[3];
	return (result);
}

void main() {
	mat4 model = mat4(1.0);
	// model = translate(model, vec3(-0.7, 1.0, 0.0) * canvasScale);
	model = scale(model, vec3(canvasScale / screenSize.x,  invertY * canvasScale / screenSize.y, 1.0)); // TODO: verify invertY correctness
	model = translate(model, vec3(-mapLength * tileSize / 2, -mapLength * tileSize / 2, 0.0));
	// model = scale(model, vec3(canvasScale / 500.0, -canvasScale / 500.0, 1.0));
	// model = scale(model, vec3(canvasScale, -canvasScale, 1.0));
	// model = translate(model, vec3(1, -1, 1) * vec3(center * 16.0, 0.0));
	model = translate(model, vec3(center * tileSize, 0.0));
	// model = translate(model, vec3(mapPosition.x * tileSize - textureOffset.x * 2.0 * textureScale.x,
	//                               mapPosition.y * tileSize - textureOffset.y * 2.0 * textureScale.y,
	//                               0.0));
	model = translate(model, vec3(mapPosition.x * tileSize, mapPosition.y * tileSize, 0.0));
	model = translate(model, vec3(spriteSize / 2, 0.0));
	model = rotate(model, radians(spriteDegrees), vec3(0.0, 0.0, 1.0));
	model = translate(model, vec3(-spriteSize / 2, 0.0));
	model = translate(model, vec3(-textureOffset.x * 2.0 * textureScale.x, -textureOffset.y * 2.0 * textureScale.y, 0.0));
	// model = scale(model, vec3(0.5, 0.5, 0.5));
	// model = translate(model, vec3(-50, 0.0, 0.0));

	// model = scale(model, vec3(screenSize, 1.0));

	// float tfactor = 4.0 / canvasScale;
	// model = translate(model, vec3((mapPosition.x * 16.0 - textureOffset.x) * tfactor, (mapPosition.y * 16.0 - textureOffset.y) * tfactor, 0.0));

	// model = scale(model, vec3(1.0, invertY, 1.0));

	// model = scale(model, vec3(screenSize.x, screenSize.y, 1.0));

	// model = rotate(model, radians(spriteDegrees), vec3(0.0, 0.0, 1.0));
	// model = translate(model, vec3(0.5 * atlasSize.x, 0.5 * atlasSize.y, 0.0));
	// model = rotate(model, radians(45.0), vec3(0.0, 0.0, 1.0));
	// model = translate(model, vec3(-0.5 * atlasSize.x, -0.5 * atlasSize.y, 0.0));

	// model = scale(model, vec3(atlasSize.x * textureScale.x * canvasScale / 2.0, atlasSize.y * textureScale.y * canvasScale / 2.0, 2.0));
	// const float A = 1.0;
	// model = scale(model, vec3(atlasSize.x * textureScale.x * canvasScale / 2.0, atlasSize.y * textureScale.y * canvasScale / 2.0, 2.0 * A) / A);
	// model = scale(model, vec3(0.5, 0.5, 1.0));

	// model = scale(model, vec3(1 / screenSize.x, 1 / screenSize.y, 1.0));

	// model = scale(model, vec3(atlasSize / screenSize, 1.0));
	// model = scale(model, vec3(screenSize, 1.0));
	model = scale(model, vec3(atlasSize, 1.0));
	// model = translate(model, vec3(0.5, 0.5, 0.0));
	// model = rotate(model, radians(45.0), vec3(0.0,0.0, 1.0));
	// model = translate(model, vec3(-0.5, -0.5, 0.0));
	// model = scale(model, vec3(screenSize, 1.0));

	texCoords = inTexCoords;
	// texCoords = position;
	spriteColor = inSpriteColor;
	specialPosition = inSpecialPosition;

	// gl_Position = projection * model * vec4(position, 0.0, 1.0);
	gl_Position = model * vec4(position, 0.0, 1.0);
	// gl_Position = projection * model * vec4(0.5, 0.5, 0.0, 1.0);
	// gl_Position = vec4(position, 0.0, 1.0);
	// gl_Position = model * projection * vec4(position, 0.0, 1.0);
	// gl_Position = projection * vec4(position, 0.0, 1.0);
}
