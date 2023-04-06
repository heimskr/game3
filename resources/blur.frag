// Credit: https://github.com/Jam3/glsl-fast-gaussian-blur

#version 420 core

precision highp float;

// in vec2 pos;           // screen position [-1, +1]
out vec4 FragColor;    // fragment output color
uniform sampler2D txr; // texture to blur
uniform float xs, ys;  // texture resolution
uniform float r;       // blur radius
uniform int axis;

vec4 blur9(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
	vec4 color = vec4(0.0);
	vec2 off1 = vec2(1.3846153846) * direction;
	vec2 off2 = vec2(3.2307692308) * direction;
	color += texture2D(image, uv) * 0.2270270270;
	color += texture2D(image, uv + (off1 / resolution)) * 0.3162162162;
	color += texture2D(image, uv - (off1 / resolution)) * 0.3162162162;
	color += texture2D(image, uv + (off2 / resolution)) * 0.0702702703;
	color += texture2D(image, uv - (off2 / resolution)) * 0.0702702703;
	return color;
}

void main() {
	vec2 resolution = vec2(xs, ys);
	vec2 uv = vec2(gl_FragCoord.xy / resolution.xy);
	// uv.y = 1.0 - uv.y;
	FragColor = blur9(txr, uv, resolution, axis == 0? vec2(r, 0.0) : vec2(0.0, r));
	// FragColor = vec4(1.0, 0.0, 1.0, 1.0);
	// FragColor = texture(txr, pos);
}
