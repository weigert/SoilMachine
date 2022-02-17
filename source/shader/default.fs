#version 430 core

in vec3 ex_FragPos;
flat in vec3 ex_Normal;
flat in vec4 ex_Color;
flat in uint ex_Index;
flat in vec4 K;
in vec4 ex_Shadow;

uniform sampler2D shadowmap;

uniform float lightstrength;
uniform vec3 lightcolor;
uniform vec3 lightpos;
uniform vec3 lookdir;


out vec4 fragColor;

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

vec4 phong() {

  float ambient = K.x;
  float diffuse = K.y*clamp(dot(ex_Normal, normalize(lightpos)), 0.0, 1.0);
  float spec = K.z*pow(max(dot(normalize(lookdir), normalize(reflect(lightpos, ex_Normal))), 0.0), K.w);

  return vec4(lightcolor*lightstrength*((1.0f-0.8*shade())*(diffuse + spec) + ambient ), 1.0f);

}


void main(void) {
  fragColor = vec4((phong()*ex_Color).xyz, ex_Color.w);
}
