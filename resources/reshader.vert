#version 430 core

precision mediump float;

layout (location = 0) in vec4 vertex;
out vec2 pos;

uniform mat4 model;
uniform mat4 projection;

// attribute vec2 position;

void main() {
	// const float pan = 0.0;
	// gl_Position = vec4(2.0 * (position - vec2(0.5, 0.5)) - vec2(pan, pan), 1, 1);
	// gl_Position = vec4(position, 1.0, 1.0);
	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
    pos = gl_Position.xy;
}
