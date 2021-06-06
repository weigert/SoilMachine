#version 430 core

in vec2 ex_Tex;
out vec4 fragColor;

uniform sampler2D imageTexture;
uniform sampler2D depthTexture;

uniform vec3 skycolor;

void main(){

    fragColor = texture(imageTexture, ex_Tex);

    float depthVal = clamp(texture(depthTexture, ex_Tex).r, 0.0, 1.0);
    fragColor = mix(fragColor, vec4(skycolor, 1), pow(depthVal,2));
  //  if(depthVal < 1.0)  //If it is a visible thing...
  //    fragColor = mix(fragColor, vec4(1.0), pow(3*(depthVal-0.55),2));  //White Fog Color!

}
