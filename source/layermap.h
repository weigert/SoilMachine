/*

layermap datastructure:
Continuous Run-Length Encoded 2.99D Height-Map using a Memory Pool!

Should allow for fast queries. Then I need a vertex pool which allows for visualization
of this thing.

Also needs a method for visualization -> vertexpool

Need a datastructure that contains parameters which we can then query based on an enumerator.

*/

#include "FastNoiseLite.h"
#include <glm/glm.hpp>

/*
enum SurfType {
  AIR, ROCK, SOIL, WATER
};

struct sec {

sec* next = NULL;     //Element Above
sec* prev = NULL;     //Element Below

SurfType type = AIR;  //Type of Surface Element
double size = 0.0f;   //Run-Length of Element
double floor = 0.0f;  //Cumulative Height at Bottom

};

class secpool {

};
*/

//We need a section pool

using namespace glm;

class Layermap {

const int SEED = 1;
ivec2 dim;
float* dat;
const double scale = 50.0f;

void meshpool(Vertexpool<Vertex>&);

double height(vec2 pos){

  if(pos.x >= dim.x || pos.y >= dim.y){
    std::cout<<"Height Request @ Pos: Out-Of-Bounds Exception"<<std::endl;
    return 0.0f;
  }

  ivec2 p = floor(pos);
  vec2 w = fract(pos);

  //Bilinear Interpolation
  double h = 0.0f;
  h += (1.0-w.x)*(1.0-w.y)*dat[p.x*dim.y+p.y];
  h += (1.0-w.x)*w.y*dat[(p.x+1)*dim.y+p.y];
  h += w.x*(1.0-w.y)*dat[p.x*dim.y+(p.y+1)];
  h += w.x*w.y*dat[(p.x+1)*dim.y+(p.y+1)];
  return h;

}

vec3 normal(ivec2 pos){

  vec3 n = vec3(0);
  vec3 p = vec3(pos.x, scale*dat[pos.x*dim.y+pos.y], pos.y);
  int k = 0;

  if(pos.x > 0 && pos.y > 0){
    vec3 b = vec3(pos.x-1, scale*dat[(pos.x-1)*dim.y+pos.y], pos.y);
    vec3 c = vec3(pos.x, scale*dat[pos.x*dim.y+(pos.y-1)], pos.y-1);
    n += cross(c-p, b-p);
    k++;
  }

  if(pos.x > 0 && pos.y < dim.y - 1){
    vec3 b = vec3(pos.x-1, scale*dat[(pos.x-1)*dim.y+pos.y], pos.y);
    vec3 c = vec3(pos.x, scale*dat[pos.x*dim.y+(pos.y+1)], pos.y+1);
    n -= cross(c-p, b-p);
    k++;
  }

  if(pos.x < dim.x-1 && pos.y > 0){
    vec3 b = vec3(pos.x+1, scale*dat[(pos.x+1)*dim.y+pos.y], pos.y);
    vec3 c = vec3(pos.x, scale*dat[pos.x*dim.y+(pos.y-1)], pos.y-1);
    n -= cross(c-p, b-p);
    k++;
  }

  if(pos.x < dim.x-1 && pos.y < dim.y-1){
    vec3 b = vec3(pos.x+1, scale*dat[(pos.x+1)*dim.y+pos.y], pos.y);
    vec3 c = vec3(pos.x, scale*dat[pos.x*dim.y+(pos.y+1)], pos.y+1);
    n += cross(c-p, b-p);
    k++;
  }

  return normalize(n/(float)k);

}

public:

Layermap(ivec2 _dim){

  dim = _dim;

  FastNoiseLite noise;
  noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  noise.SetFractalType(FastNoiseLite::FractalType_FBm);
  noise.SetFractalOctaves(8.0f);
  noise.SetFractalLacunarity(2.0f);
  noise.SetFractalGain(0.6f);
  noise.SetFrequency(1.0);

  dat = new float[dim.x*dim.y];
  float min, max = 0.0;

  for(int i = 0; i < dim.x; i++){
    for(int j = 0; j < dim.y; j++){
      dat[i*dim.y+j] = noise.GetNoise((float)(i)*(1.0f/dim.x), (float)(j)*(1.0f/dim.y), (float)(SEED%1000));
      if(dat[i*dim.y+j] > max) max = dat[i*dim.y+j];
      if(dat[i*dim.y+j] < min) min = dat[i*dim.y+j];
    }
  }

  for(int i = 0; i < dim.x; i++){
    for(int j = 0; j < dim.y; j++){
      dat[i*dim.y+j] = (dat[i*dim.y+j] - min)/(max - min);
    }
  }

}

Layermap(ivec2 _dim, Vertexpool<Vertex>& vertexpool):Layermap(_dim){
  meshpool(vertexpool);
}

//sec** dat;            //Actual Section Data


};

void Layermap::meshpool(Vertexpool<Vertex>& vertexpool){

  uint* section = vertexpool.section(dim.x*dim.y, 0, glm::vec3(0));

  for(int i = 0; i < dim.x; i++){
  for(int j = 0; j < dim.y; j++){

    vertexpool.fill(section, i*dim.y+j,
      vec3(i, scale*dat[i*dim.y+j], j),
      normal(ivec2(i,j)),
      vec4(0.5, 0.5, 0.5, 1.0)
    );

  }}

  for(int j = 0; j < dim.y-1; j++){
  for(int i = 0; i < dim.x-1; i++){

    vertexpool.indices.push_back(i*dim.y+j);
    vertexpool.indices.push_back(i*dim.y+(j+1));
    vertexpool.indices.push_back((i+1)*dim.y+j);

    vertexpool.indices.push_back((i+1)*dim.y+j);
    vertexpool.indices.push_back(i*dim.y+(j+1));
    vertexpool.indices.push_back((i+1)*dim.y+(j+1));

  }}

  vertexpool.resize(section, vertexpool.indices.size());
  vertexpool.index();
  vertexpool.update();

}
