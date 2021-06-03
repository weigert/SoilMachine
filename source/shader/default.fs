#version 430 core

in vec3 ex_FragPos;
in vec3 ex_Normal;
in vec4 ex_Color;

out vec4 fragColor;

void main(void) {
  fragColor = ex_Color;
}
