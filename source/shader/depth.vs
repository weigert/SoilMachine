#version 430 core

layout(location = 0) in vec3 in_Position;

uniform mat4 dmvp;

void main(void) {
	gl_Position = dmvp * vec4(in_Position, 1.0f);
}
