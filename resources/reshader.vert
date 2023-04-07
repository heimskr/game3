#version 430 core

//*
precision mediump float;

layout (location = 0) in vec4 vertex;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 projection;

// attribute vec2 position;

void main() {
	// const float pan = 0.0;
	// gl_Position = vec4(2.0 * (position - vec2(0.5, 0.5)) - vec2(pan, pan), 1, 1);
	// gl_Position = vec4(position, 1.0, 1.0);
	gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0) + vec4(0.25, 0.25, 0.0, 0.0);
	gl_Position = vec4(vertex.xy, 0.0, 1.0) - vec4(0.5, 0.5, 0.0, 0.0);
	// gl_Position = projection * model * vertex;
    TexCoord = gl_Position.xy;
}
//*/


/*

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoord;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 projection;

void main() {
	// const float pan = 0.0;
	// gl_Position = vec4(2.0 * (position - vec2(0.5, 0.5)) - vec2(pan, pan), 1, 1);
	gl_Position = projection * model * vec4(aPosition, 1.0);
	// gl_Position = vec4(aPosition, 1.0);
	TexCoord = aTexCoord;
}
*/

/*
in vec2 position;

void main() {
  gl_Position = vec4(position, 1, 1);
}
*/