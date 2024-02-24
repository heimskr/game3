#version 410 core

layout(location = 0) in dvec2  position;
layout(location = 1) in dvec2  mapPosition;   // options.{x, y}
layout(location = 2) in dvec2  spriteOffset;  // options.offset{X, Y}
layout(location = 3) in dvec2  textureScale;  // options.scale{X, Y}
layout(location = 4) in double invertY;       // options.invertY? -1 : 1
layout(location = 5) in double spriteDegrees; // options.angle
layout(location = 6) in dvec4  inSpriteColor; // options.color
layout(location = 7) in dvec2  spriteSize;    // options.size{X, Y}
layout(location = 8) in dvec4  inSpecialPosition;

flat out dvec2 texCoords;
flat out dvec4 spriteColor;
flat out dvec4 specialPosition;

uniform dmat4 projection;
uniform vec2 atlasSize; // texture_{width, height}
uniform float canvasScale;
uniform dvec2 screenSize;
uniform dvec2 center;
uniform float tileSize;
uniform float mapLength;

dmat4 translate(dmat4 matrix, dvec3 delta) {
	matrix[3] = matrix[0] * delta[0] + matrix[1] * delta[1] + matrix[2] * delta[2] + matrix[3];
	return matrix;
}

dmat4 scale(dmat4 matrix, dvec3 scale_vec) {
	matrix[0] *= scale_vec[0];
	matrix[1] *= scale_vec[1];
	matrix[2] *= scale_vec[2];
	return matrix;
}

dvec3 normalize3(dvec3 vec) {
	return vec * inversesqrt(dot(vec, vec));
}

dmat4 rotate(dmat4 matrix, float angle, dvec3 axis) {
	double c = cos(angle);
	double s = sin(angle);
	axis = normalize(axis);
	dvec3 temp = (1.0 - c) * axis;

	dmat4 rotation = dmat4(0.0);
	rotation[0][0] = c + temp[0] * axis[0];
	rotation[1][0] = temp[0] * axis[1] + s * axis[2];
	rotation[2][0] = temp[0] * axis[2] - s * axis[1];
	rotation[0][1] = temp[1] * axis[0] - s * axis[2];
	rotation[1][1] = c + temp[1] * axis[1];
	rotation[2][1] = temp[1] * axis[2] + s * axis[0];
	rotation[0][2] = temp[2] * axis[0] + s * axis[1];
	rotation[1][2] = temp[2] * axis[1] - s * axis[0];
	rotation[2][2] = c + temp[2] * axis[2];

	dmat4 result = dmat4(0.0);
	result[0] = matrix[0] * rotation[0][0] + matrix[1] * rotation[1][0] + matrix[2] * rotation[2][0];
	result[1] = matrix[0] * rotation[0][1] + matrix[1] * rotation[1][1] + matrix[2] * rotation[2][1];
	result[2] = matrix[0] * rotation[0][2] + matrix[1] * rotation[1][2] + matrix[2] * rotation[2][2];
	result[3] = matrix[3];
	return result;
}

void main() {
	dmat4 model = dmat4(1.0);
	model = scale(model, dvec3(canvasScale / screenSize.x,  invertY * canvasScale / screenSize.y, 1.0)); // TODO: verify invertY correctness
	model = translate(model, dvec3(-mapLength * tileSize / 2, -mapLength * tileSize / 2, 0.0));
	model = translate(model, dvec3(center * tileSize, 0.0));
	model = translate(model, dvec3(mapPosition.x * tileSize, mapPosition.y * tileSize, 0.0));
	model = translate(model, dvec3(spriteSize / 2, 0.0));
	model = rotate(model, radians(float(spriteDegrees)), dvec3(0.0, 0.0, 1.0));
	model = translate(model, dvec3(-spriteSize / 2, 0.0));
	model = translate(model, dvec3(-spriteOffset.x * 2.0 * textureScale.x, -spriteOffset.y * 2.0 * textureScale.y, 0.0));
	model = scale(model, dvec3(atlasSize * textureScale, 1.0));

	texCoords = position;
	spriteColor = inSpriteColor;
	specialPosition = inSpecialPosition;

	gl_Position = vec4(model * dvec4(position, 0.0, 1.0)); // All the doubles for nothing?
}
