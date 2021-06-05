/*
================================================================================
                          Layermap Data Structure
================================================================================

Continuous Run-Length Encoded Heightmap with Memory Pooled RLE Components
Utilizes a vertexpool to avoid remeshing when updating heightmap information

*/

#include "include/FastNoiseLite.h"
#include <glm/glm.hpp>
using namespace glm;

#include "surface.h"

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
public:


  sec* start = NULL;
  int size;
  deque<sec*> free;
  secpool(){}

  secpool(const int N){
    reserve(N);
  }

  void reserve(const int N){
    start = new sec[N];
    for(int i = 0; i < N; i++)
      free.push_front(start+i);
    size = N;
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
double remove(ivec2, double);               //Remove Layer at Position
sec* top(ivec2 pos){
  return dat[pos.x*dim.y+pos.y];
}

//Meshing
uint* section = NULL;                     //Vertexpool Section Pointer
void meshpool(Vertexpool<Vertex>&);       //Mesh based on Vertexpool
void update(ivec2, Vertexpool<Vertex>&);  //Update Vertexpool at Position (No Remesh)

public:

//Constructors
Layermap(int SEED, ivec2 _dim){

  dim = _dim;
  dat = new sec*[dim.x*dim.y];      //Array of Section Pointers
  pool.reserve(dim.x*dim.y*15);

  //Set the Height!
  FastNoiseLite noise;
  noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
  noise.SetFractalType(FastNoiseLite::FractalType_FBm);
  noise.SetFractalOctaves(7.0f);
  noise.SetFractalLacunarity(2.0f);
  noise.SetFractalGain(0.5f);
  noise.SetFrequency(1.0);

  //Add a first layer!
  for(int i = 0; i < dim.x; i++){
  for(int j = 0; j < dim.y; j++){

    dat[i*dim.y+j] = NULL; //Initialize to Null

    //Compute Height Value
  //  double h = 0.5f+noise.GetNoise((float)(i)*(1.0f/dim.x), (float)(j)*(1.0f/dim.y), (float)(SEED%1000));
  //  if(h > 0.0) add(ivec2(i, j), pool.get(h, SAND));

    double h = 0.2f+noise.GetNoise((float)(i)*(1.0f/dim.x), (float)(j)*(1.0f/dim.y), (float)(SEED%1000));
    if(h > 0.0) add(ivec2(i, j), pool.get(h, ROCK));

  //  h = 0.5f*noise.GetNoise((float)(i)*(1.0f/dim.x), (float)(j)*(1.0f/dim.y), (float)((SEED+10)%1000));
  //  if(h > 0.0) add(ivec2(i, j), pool.get(h, SAND));

    //Second Layer!
  //  h = noise.GetNoise((float)(i)*(1.0f/dim.x), (float)(j)*(1.0f/dim.y), (float)((SEED+50)%1000));
  //  if(h > 0.0) add(ivec2(i, j), pool.get(h, SOIL));

  }}

}

Layermap(int SEED, ivec2 _dim, Vertexpool<Vertex>& vertexpool):Layermap(SEED, _dim){
  meshpool(vertexpool);
}

};

void Layermap::add(ivec2 pos, sec* E){

  if(dat[pos.x*dim.y+pos.y] == NULL){
    dat[pos.x*dim.y+pos.y] = E;
    return;
  }

  //Same Type
  if(dat[pos.x*dim.y+pos.y]->type == E->type){
    dat[pos.x*dim.y+pos.y]->size += E->size;
    pool.unget(E);
    return;
  }

  //Add Element
  dat[pos.x*dim.y+pos.y]->next = E;
  E->prev = dat[pos.x*dim.y+pos.y];
  E->floor = height(pos);
  dat[pos.x*dim.y+pos.y] = E;   //E->prev = dat[pos.x*dim.y+pos.y]; //Reference Previous Element

/*
  //Ordering
  sec* cur = dat[pos.x*dim.y+pos.y];
  while(cur->prev != NULL){

    if(pdict[cur->type].density >= pdict[cur->prev->type].density){

      dat[pos.x*dim.y+pos.y]->prev->size += dat[pos.x*dim.y+pos.y]->size;

      if(cur->prev->type != cur->type)
        dat[pos.x*dim.y+pos.y]->prev->type = dat[pos.x*dim.y+pos.y]->type;

      dat[pos.x*dim.y+pos.y] = dat[pos.x*dim.y+pos.y]->prev;
      pool.unget(cur);

      cur = dat[pos.x*dim.y+pos.y];

    }
    else cur = cur->prev;

  }
*/

}

double Layermap::remove(ivec2 pos, double h){

  if(dat[pos.x*dim.y+pos.y] == NULL)
    return 0.0; //Return Remaining Reight

  double diff = h - dat[pos.x*dim.y+pos.y]->size;
  dat[pos.x*dim.y+pos.y]->size -= h;

  if(diff >= 0.0){
    sec* E = dat[pos.x*dim.y+pos.y];
    dat[pos.x*dim.y+pos.y] = E->prev; //May be NULL
    pool.unget(E);
    return diff;
  }
  else return 0.0;

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
    pdict[surface(p)].color
  );

}
