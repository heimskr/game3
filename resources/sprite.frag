#version 330 core

// Credit: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/7.in_practice/3.2d_game/0.full_source/sprite.fs

// layout(location = 0) out vec4 color;

in vec2 texCoords;
in vec4 spriteColor;
in vec4 specialPosition;
out vec4 color;

uniform sampler2D sprite;
// uniform vec4 spriteColor;
// uniform vec4 texturePosition;
// uniform float divisor;

void main() {
	color = spriteColor * texture(sprite, texCoords);
	// color.a = 1.0;
	// color.r = 1.0;

	// color = texture(sprite, texCoords);
	// gl_FragColor = texture(sprite, texCoords);
	// color = vec4(1.0, 1.0, 0.0, 1.0);
	// color = vec4(1.0, 1.0, 0.0, 1.0);

	if (texCoords.x < specialPosition.x || texCoords.y < specialPosition.y || specialPosition.x + specialPosition.z < texCoords.x || specialPosition.y + specialPosition.w < texCoords.y) {
		// discard;
	// } else {
		// color.r /= divisor;
		// color.g /= divisor;
		// color.b /= divisor;
	}
}
