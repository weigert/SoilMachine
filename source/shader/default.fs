#version 430 core

in vec3 ex_FragPos;
in vec3 ex_Normal;
in vec4 ex_Color;
in vec4 ex_Shadow;

out vec4 fragColor;


void main(void) {
  fragColor = vec4(ex_Color.xyz, 1.0f);
}
