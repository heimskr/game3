// Credit: https://www.shadertoy.com/view/ltScRG
// 16x acceleration of https://www.shadertoy.com/view/4tSyzy by applying gaussian at intermediate MIPmap level.

#version 330 core

in vec2 TexCoords;
out vec4 color;    // fragment output color
uniform sampler2D txr; // texture to blur
uniform vec2 resolution;  // texture resolution
uniform vec2 radius;      // blur radius

const int samples = 20,
          LOD = 2,         // gaussian done on MIPmap at scale LOD
          sLOD = 1 << LOD; // tile size = 2^LOD

const float sigma = float(samples) * 0.25;

float gaussian(vec2 i) {
    return exp(-0.5 * dot(i /= sigma, i)) / (6.28 * sigma * sigma);
}

vec4 blur(sampler2D sp, vec2 U, vec2 scale) {
    vec4 blurred = vec4(0);
    int s = samples / sLOD;

    for (int i = 0; i < s * s; i++) {
        vec2 d = vec2(i % s, i / s) * float(sLOD) - float(samples) / 2.0;
        blurred += gaussian(d) * textureLod( sp, U + scale * d , float(LOD) );
    }

    return blurred / blurred.a;
}

void main() {
	vec2 uv = TexCoords;
	uv.y = 1.0 - uv.y;
    color = blur(txr, uv, 1.0 / resolution);
}
