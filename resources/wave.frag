#version 330 core

// [2TC 15] Water2D
// Copyleft {c} 2015 Michael Pohoreski

in vec2 TexCoords;
out vec4 color;

uniform sampler2D txr;
uniform float time;
uniform vec2 resolution;
uniform vec4 colorMultiplier;

const float speed     = 0.002;
const float frequency = 128.0;

vec2 shift(vec2 p) {
	float d = time * speed;
    vec2 f = frequency * (p + d);
    return cos(vec2(
       cos(f.x - f.y) * cos(f.y),
       sin(f.x + f.y) * sin(f.y)
	));
}

void main() {
	vec2 m = 128 / resolution.xy;
	vec2 r = TexCoords * m;
    vec2 p = shift(r);
    vec2 q = shift(r + 1.0);
	float amplitude = 1 / 1024.0;
    vec2 s = r + amplitude * (p - q);
	s /= m;
    s.y = 1.0 - s.y;
    color = texture(txr, s) * colorMultiplier;
}
