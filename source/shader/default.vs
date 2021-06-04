#version 430 core

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Color;

uniform mat4 model;
uniform mat4 vp;
uniform mat4 dbvp;

uniform sampler2D shadowmap;

uniform float lightstrength;
uniform vec3 lightcolor;
uniform vec3 lightpos;
uniform vec3 lookdir;

out vec3 ex_FragPos;
out vec3 ex_Normal;
out vec4 ex_Color;
out vec4 ex_Shadow;

float gridSample(int size){
  float shadow = 0.0;
  float cur = ex_Shadow.z;

  float m = 1 - dot(ex_Normal, normalize(lightpos));
  float bias = mix(0.005, 0.001*m, pow(m, 5));

	for(int x = -size; x <= size; ++x){
      for(int y = -size; y <= size; ++y){
          float near = texture(shadowmap, ex_Shadow.xy + vec2(x, y) / textureSize(shadowmap, 0)).r;
					shadow += cur-bias > near ? 1.0 : 0.0;
      }
  }

  float norm = (2*size+1)*(2*size+1);
  return shadow/norm;
}

float shade(){
    float shadow = 0.0;
		int size = 1;

    if(greaterThanEqual(ex_Shadow.xy, vec2(0.0f)) == bvec2(true) && lessThanEqual(ex_Shadow.xy, vec2(1.0f)) == bvec2(true))
      shadow = gridSample(size);

		return shadow;
}


vec4 gouraud(){

	float diffuse = clamp(dot(in_Normal, normalize(lightpos)), 0.2, 0.8);
	float ambient = 0.3;
	float spec = 0.8*pow(max(dot(normalize(lookdir), normalize(reflect(lightpos, in_Normal))), 0.0), 32.0);

	return vec4(lightcolor*lightstrength*((1.0f)*(diffuse + spec) + ambient ), 1.0f);

}

void main(void) {

	ex_FragPos = (model * vec4(in_Position, 1.0f)).xyz;
	ex_Shadow = dbvp* vec4(ex_FragPos, 1.0f);
	ex_Normal = in_Normal;	//Pass Normal
	ex_Color = gouraud()*in_Color;

	gl_Position = vp * vec4(ex_FragPos, 1.0f);

}
