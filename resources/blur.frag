// Credit: https://stackoverflow.com/a/64845819

#version 420 core

in vec2 pos;           // screen position [-1, +1]
out vec4 FragColor;    // fragment output color
uniform sampler2D txr; // texture to blur
uniform float xs, ys;  // texture resolution
uniform float r;       // blur radius
uniform int axis;

void main() {
	float x, y, d, w, w0;
	float rr = r * r;
	vec2 p = 0.5 * (vec2(1.0, 1.0) + pos);
	vec4 col = vec4(0.0, 0.0, 0.0, 0.0);
	w0 = 0.5135 / pow(r, 0.96);

	if (axis == 0) {
		for (d = 1.0 / xs, x = -r, p.x += x * d; x <= r; x++, p.x += d) {
			w = w0 * exp((-x * x) / (2.0 * rr));
			col += texture(txr, p) * w;
		}
	} else if (axis == 1) {
		for (d = 1.0 / ys, y = -r, p.y += y * d; y <= r; y++, p.y += d) {
			w = w0 * exp((-y * y) / (2.0 * rr));
			col += texture(txr, p) * w;
		}
	}

	// FragColor = col;


	FragColor = texture(txr, p);
}
