/*
================================================================================
                          Layermap Data Structure
================================================================================

Continuous Run-Length Encoded Heightmap with Memory Pooled RLE Components
Utilizes a vertexpool to avoid remeshing when updating heightmap information

*/

#include "FastNoiseLite.h"
#include <glm/glm.hpp>
using namespace glm;

// Surface Type Enumerator

enum SurfType {
  AIR, ROCK, SOIL, WATER
};

vec4 SurfColor(SurfType type){
  if(type == ROCK) return vec4(0.5, 0.5, 0.5, 1.0);
  if(type == SOIL) return vec4(0.0, 0.5, 0.0, 1.0);
  if(type == WATER) return vec4(0.0, 0.0, 0.5, 1.0);
  return vec4(1.0, 0.0, 0.0, 0.0);
}

// RLE Section and Memory Pool

struct sec {

sec* next = NULL;     //Element Above
sec* prev = NULL;     //Element Below

SurfType type = AIR;  //Type of Surface Element
double size = 0.0f;   //Run-Length of Element
double floor = 0.0f;  //Cumulative Height at Bottom

sec(){}
sec(double s, SurfType t){
  size = s;
  type = t;
}

void reset(){
  next = NULL;
  prev = NULL;
  type = AIR;
  size = 0.0f;
  floor = 0.0f;
}

};

class secpool {

  sec* start = NULL;
  deque<sec*> free;

public:
  secpool(){}

  secpool(const int N){
    reserve(N);
  }

  void reserve(const int N){
    start = new sec[N];
    for(int i = 0; i < N; i++)
      free.push_front(start+i);
  }

  //Construct-in-place and fetch
  template<typename... Args>
  sec* get(Args && ...args){
    if(free.empty()){
      std::cout<<"Memory Pool Out-Of-Elements"<<std::endl;
      return NULL;
    }
    sec* E = free.back();
    try{ new (E)sec(forward<Args>(args)...); }
    catch(...) { throw; }
    free.pop_back();
    return E;
  }

  void unget(sec* E){
    E->reset();
    free.push_front(E);
  }

};

//Layermap Data Structure with Queries

class Layermap {

public:

ivec2 dim;                                //Size
sec** dat;                                //Data
secpool pool;                             //Data Pool

//Queries
double height(ivec2);                     //Query Height at Discrete Position
double height(vec2);                      //Query Height at Position (Bilinear Interpolation)
vec3 normal(ivec2);                       //Normal Vector at Position
vec3 normal(vec2);                        //Normal Vector at Position (Bilinear Interpolation)
SurfType surface(ivec2);                  //Surface Type at Position

//Modifiers
void add(ivec2, sec*);                    //Add Layer at Position
void remove(ivec2, double);               //Remove Layer at Position
sec* top(ivec2 pos){
  return dat[pos.x*dim.y+pos.y];
}

//Meshing
uint* section = NULL;                     //Vertexpool Section Pointer
void meshpool(Vertexpool<Vertex>&);       //Mesh based on Vertexpool
void update(ivec2, Vertexpool<Vertex>&);  //Update Vertexpool at Position (No Remesh)

public:

//Constructors
Layermap(ivec2 _dim){

  dim = _dim;
  dat = new sec*[dim.x*dim.y];      //Array of Section Pointers
  pool.reserve(dim.x*dim.y*2);

  //Set the Height!
  FastNoiseLite noise;
  noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  noise.SetFractalType(FastNoiseLite::FractalType_FBm);
  noise.SetFractalOctaves(6.0f);
  noise.SetFractalLacunarity(2.0f);
  noise.SetFractalGain(0.5f);
  noise.SetFrequency(1.0);

  //Add a first layer!
  for(int i = 0; i < dim.x; i++){
  for(int j = 0; j < dim.y; j++){

    dat[i*dim.y+j] = NULL; //Initialize to Null

    //Compute Height Value
    double h = 0.5f+noise.GetNoise((float)(i)*(1.0f/dim.x), (float)(j)*(1.0f/dim.y), (float)(SEED%1000));
    if(h > 0.0) add(ivec2(i, j), pool.get(h, ROCK));

    //Second Layer!
  //  h = noise.GetNoise((float)(i)*(1.0f/dim.x), (float)(j)*(1.0f/dim.y), (float)((SEED+50)%1000));
  //  if(h > 0.0) add(ivec2(i, j), pool.get(h, SOIL));

  }}

}

Layermap(ivec2 _dim, Vertexpool<Vertex>& vertexpool):Layermap(_dim){
  meshpool(vertexpool);
}

};

void Layermap::add(ivec2 pos, sec* E){

  if(dat[pos.x*dim.y+pos.y] == NULL){
    dat[pos.x*dim.y+pos.y] = E;
  }
  else if(dat[pos.x*dim.y+pos.y] != NULL){

    //Check for same type
    if(dat[pos.x*dim.y+pos.y]->type == E->type){

      dat[pos.x*dim.y+pos.y]->size += E->size;
      pool.unget(E); //Return the Element!

    }

    //Add New Element
    else{

      dat[pos.x*dim.y+pos.y]->next = E; //Reference Next Element
      E->prev = dat[pos.x*dim.y+pos.y]; //Reference Previous Element

      E->floor = height(pos);
      dat[pos.x*dim.y+pos.y] = E;

    }

  }

}

void Layermap::remove(ivec2 pos, double h){

  if(dat[pos.x*dim.y+pos.y] == NULL)
    return;

  dat[pos.x*dim.y+pos.y]->size -= h;

  if(dat[pos.x*dim.y+pos.y]->size < 0.0){
    sec* E = dat[pos.x*dim.y+pos.y];
    dat[pos.x*dim.y+pos.y] = E->prev; //May be NULL
    pool.unget(E);
  }

}

vec3 Layermap::normal(ivec2 pos){

  vec3 n = vec3(0);
  vec3 p = vec3(pos.x, scale*height(pos), pos.y);
  int k = 0;

  if(pos.x > 0 && pos.y > 0){
    vec3 b = vec3(pos.x-1, scale*height(pos-ivec2(1,0)), pos.y);
    vec3 c = vec3(pos.x, scale*height(pos-ivec2(0,1)), pos.y-1);
    n += cross(c-p, b-p);
    k++;
  }

  if(pos.x > 0 && pos.y < dim.y - 1){
    vec3 b = vec3(pos.x-1, scale*height(pos-ivec2(1,0)), pos.y);
    vec3 c = vec3(pos.x, scale*height(pos+ivec2(0,1)), pos.y+1);
    n -= cross(c-p, b-p);
    k++;
  }

  if(pos.x < dim.x-1 && pos.y > 0){
    vec3 b = vec3(pos.x+1, scale*height(pos+ivec2(1,0)), pos.y);
    vec3 c = vec3(pos.x, scale*height(pos-ivec2(0,1)), pos.y-1);
    n -= cross(c-p, b-p);
    k++;
  }

  if(pos.x < dim.x-1 && pos.y < dim.y-1){
    vec3 b = vec3(pos.x+1, scale*height(pos+ivec2(1,0)), pos.y);
    vec3 c = vec3(pos.x, scale*height(pos+ivec2(0,1)), pos.y+1);
    n += cross(c-p, b-p);
    k++;
  }

  return normalize(n/(float)k);

}

vec3 Layermap::normal(vec2 pos){

  vec3 n = vec3(0);
  ivec2 p = floor(pos);
  vec2 w = fract(pos);

  n += (1.0f-w.x)*(1.0f-w.y)*normal(p);
  n += (1.0f-w.x)*w.y*normal(p+ivec2(1,0));
  n += w.x*(1.0f-w.y)*normal(p+ivec2(0,1));
  n += w.x*w.y*normal(p+ivec2(1,1));

  return n;

}

//Queries

SurfType Layermap::surface(ivec2 pos){
  if(dat[pos.x*dim.y+pos.y] == NULL) return AIR;
  return dat[pos.x*dim.y+pos.y]->type;
}

double Layermap::height(ivec2 pos){
  if(dat[pos.x*dim.y+pos.y] == NULL) return 0.0;
  return (dat[pos.x*dim.y+pos.y]->floor + dat[pos.x*dim.y+pos.y]->size);
}

double Layermap::height(vec2 pos){

  //std::cout<<pos.x<<" "<<pos.y<<std::endl;

  double h = 0.0f;
  ivec2 p = floor(pos);
  vec2 w = fract(pos);

  h += (1.0-w.x)*(1.0-w.y)*height(p);
  h += (1.0-w.x)*w.y*height(p+ivec2(1,0));
  h += w.x*(1.0-w.y)*height(p+ivec2(0,1));
  h += w.x*w.y*height(p+ivec2(1,1));
  return h;

}

// Meshing

void Layermap::meshpool(Vertexpool<Vertex>& vertexpool){

  section = vertexpool.section(dim.x*dim.y, 0, glm::vec3(0));

  for(int i = 0; i < dim.x; i++)
  for(int j = 0; j < dim.y; j++)
    update(ivec2(i,j), vertexpool);

  for(int i = 0; i < dim.x-1; i++){
  for(int j = 0; j < dim.y-1; j++){

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

void Layermap::update(ivec2 p, Vertexpool<Vertex>& vertexpool){

  vertexpool.fill(section, p.x*dim.y+p.y,
    vec3(p.x, scale*height(p), p.y),
    normal(p),
    SurfColor(surface(p))
  );

}
