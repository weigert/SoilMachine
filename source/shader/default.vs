#version 430 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;

uniform mat4 model;
uniform mat4 vp;

uniform float lightstrength;
uniform vec3 lightcolor;
uniform vec3 lightpos;
uniform vec3 lookdir;

out vec3 ex_FragPos;
out vec3 ex_Normal;
out vec4 ex_Color;

vec4 gouraud(){

	float diffuse = clamp(dot(in_Normal, normalize(lightpos)), 0.1, 0.9);
	float ambient = 0.1;
	float spec = 0.8*pow(max(dot(normalize(lookdir), normalize(reflect(lightpos, in_Normal))), 0.0), 32.0);

	return vec4(lightcolor*lightstrength*(diffuse + ambient + spec), 1.0f);

}

void main(void) {

	ex_FragPos = (model * vec4(in_Position, 1.0f)).xyz;
	ex_Normal = in_Normal;	//Pass Normal
	ex_Color = gouraud()*in_Color;

	gl_Position = vp * vec4(ex_FragPos, 1.0f);

}
