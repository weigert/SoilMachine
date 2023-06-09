/*
================================================================================
                          Layermap Data Structure
================================================================================

Concept: Heightmap representet as a grid. Each grid element points to the "top"
element of a double-linked-list of RLE components which contain the layer meta data.
These components are additionally memory pooled for efficiency.

Utilizes a vertexpool to avoid remeshing when updating heightmap information

A layermap section has a height, a porosity and a water fraction.
The idea is that if the water fraction is 1, then all pores are filled.
This tells me the overall saturation of water inside.
Layermap has a height, a porosity and a

Air e.g. has porosity 1

*/

//#define SOILMACHINE_MASK

#include <glm/glm.hpp>
using namespace glm;
using namespace std;

/*
================================================================================
                  RLE Linked-List Element and Memory Pool
================================================================================
*/

struct sec {

sec* next = NULL;     //Element Above
sec* prev = NULL;     //Element Below

SurfType type = soilmap["Air"];   //Type of Surface Element
double size = 0.0f;               //Run-Length of Element
double floor = 0.0f;              //Cumulative Height at Bottom
double saturation = 0.0f;         //Saturation with Water

sec(){}
sec(double s, SurfType t){
  size = s;
  type = t;
}

void reset(){
  next = NULL;
  prev = NULL;
  type = soilmap["Air"];
  size = 0.0f;
  floor = 0.0f;
  saturation = 0.0f;
}

};

class secpool {
public:

int size;               //Number of Total Elements
sec* start = NULL;      //Point to Start of Pool
deque<sec*> free;       //Queue of Free Elements

secpool(){}             //Construct
secpool(const int N){   //Construct with Size
  reserve(N);
}
~secpool(){
  free.clear();
  delete[] start;
}

//Create the Memory Pool
void reserve(const int N){
  start = new sec[N];
  for(int i = 0; i < N; i++)
    free.push_front(start+i);
  size = N;
}

//Retrieve Element, Construct in Place
template<typename... Args>
sec* get(Args && ...args){

  if(free.empty()){
    cout<<"Memory Pool Out-Of-Elements"<<endl;
    return NULL;
  }

  sec* E = free.back();
  try{ new (E)sec(forward<Args>(args)...); }
  catch(...) { throw; }
  free.pop_back();
  return E;

}

//Return Element
void unget(sec* E){
  if(E == NULL)
    return;
  E->reset();
  free.push_front(E);
}

void reset(){
  free.clear();
  for(int i = 0; i < size; i++)
    free.push_front(start+i);
}

};






















/*
================================================================================
                      Queriable Layermap Datastructure
================================================================================
*/

class Layermap {

private:

sec** dat = NULL;                         //Raw Data Grid

public:

ivec2 dim;                                //Size
secpool pool;                             //Data Pool

//Queries
double height(ivec2);                     //Query Height at Discrete Position
double height(vec2);                      //Query Height at Position (Bilinear Interpolation)
vec3 normal(ivec2);                       //Normal Vector at Position
vec3 normal(vec2);                        //Normal Vector at Position (Bilinear Interpolation)
SurfType surface(ivec2);                  //Surface Type at Position

//Modifiers
void add(ivec2, sec*);                    //Add Layer at Position
double remove(ivec2, double);             //Remove Layer at Position
sec* top(ivec2 pos){                      //Top Element at Position
  return dat[pos.x*dim.y+pos.y];
}

public:

void initialize(int SEED, ivec2 _dim){

  dim = _dim;

  //Important so Re-Callable

  pool.reset();

  if(dat != NULL) delete[] dat;
  dat = new sec*[dim.x*dim.y];      //Array of Section Pointers

  for(int i = 0; i < dim.x; i++)
  for(int j = 0; j < dim.y; j++)
    dat[i*dim.y+j] = NULL;

  //Fill 'er up

  const int MAXSEED = 10000;
  for(size_t l = 0; l < layers.size(); l++){

    const float f = (float)l/(float)layers.size();
    const int Z = SEED + f*MAXSEED;
    layers[l].init();

    //Add a first layer!
    for(int i = 0; i < dim.x; i++){
    for(int j = 0; j < dim.y; j++){

      double h = layers[l].get(vec3(i, j, Z%MAXSEED)/vec3(dim.x, dim.y, 1));
      add(ivec2(i, j), pool.get(h, layers[l].type));

    }}

  }

  // Mask the World

  #ifdef SOILMACHINE_MASK

  layers[0].scale = 1.0f;
  layers[0].bias = 0.5f;

  for(int i = 0; i < dim.x; i++)
  for(int j = 0; j < dim.y; j++){

    double maxh = layers[0].get(vec3(i, j, SEED%MAXSEED)/vec3(dim.x, dim.y, MAXSEED));
    float diff = remove(ivec2(i, j), height(ivec2(i, j)) - maxh);
    while(diff > 0) diff = remove(ivec2(i, j), height(ivec2(i, j)) - maxh);

  }

  #endif

}

//Constructors
Layermap(int SEED, ivec2 _dim){
  pool.reserve(POOLSIZE);                //Some permissible amount of RAM later...
  initialize(SEED, _dim);
}

};

void Layermap::add(ivec2 pos, sec* E){

  //Non-Element: Don't Add
  if(E == NULL)
    return;

  //Negative Size Element: Don't Add
  if(E->size <= 0){
    pool.unget(E);
    return;
  }

  //Valid Element, Empty Spot: Set Top Directly
  if(dat[pos.x*dim.y+pos.y] == NULL){
    dat[pos.x*dim.y+pos.y] = E;
    return;
  }

  //Valid Element, Previous Type Identical: Elongate
  if(dat[pos.x*dim.y+pos.y]->type == E->type){
    dat[pos.x*dim.y+pos.y]->size += E->size;
    pool.unget(E);
    return;
  }

  //Basically: A position Swap

  //Add to Water, but not equal to water
  if(dat[pos.x*dim.y+pos.y]->type == soilmap["Air"]){ //Switch with Water

  //  pool.unget(E);

    //Remove Top Element (Water)
    sec* top = dat[pos.x*dim.y+pos.y];
    dat[pos.x*dim.y+pos.y] = top->prev;

    //Add this Element
    add(pos, E);

    //Add Water Back In
    add(pos, top);


    return;

  }

  /*
  if(dat[pos.x*dim.y+pos.y]->prev != NULL)
  if(dat[pos.x*dim.y+pos.y]->prev->size < 0.01)
  if(dat[pos.x*dim.y+pos.y]->prev->type == E->type){    //Same Type: Make Taller, Remove E
    dat[pos.x*dim.y+pos.y]->prev->size += E->size;
    dat[pos.x*dim.y+pos.y]->floor += E->size;
    pool.unget(E);
    return;
  }
  */

  //Try a sorting move???

  /*
  if(dat[pos.x*dim.y+pos.y]->prev != NULL)
  if(soils[dat[pos.x*dim.y+pos.y]->type].density < soils[E->type].density)
  if(dat[pos.x*dim.y+pos.y]->prev->type == E->type){    //Same Type: Make Taller, Remove E
    dat[pos.x*dim.y+pos.y]->prev->size += E->size;
    dat[pos.x*dim.y+pos.y]->floor += E->size;
    pool.unget(E);
    return;
  }
  */

  //Add Element
  dat[pos.x*dim.y+pos.y]->next = E;
  E->prev = dat[pos.x*dim.y+pos.y];
  E->floor = height(pos);
  dat[pos.x*dim.y+pos.y] = E;

}

//Returns Amount Removed
double Layermap::remove(ivec2 pos, double h){

  //No Element to Remove
  if(dat[pos.x*dim.y+pos.y] == NULL)
    return 0.0;

  //Element Needs Removal
  if(dat[pos.x*dim.y+pos.y]->size <= 0.0){
    sec* E = dat[pos.x*dim.y+pos.y];
    dat[pos.x*dim.y+pos.y] = E->prev; //May be NULL
    pool.unget(E);
    return 0.0;
  }

  //No Removal Necessary (Note: Zero Height Elements Removed)
  if(h <= 0.0)
    return 0.0;

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
  vec3 p = vec3(pos.x, SCALE*height(pos), pos.y);
  int k = 0;

  if(pos.x > 0 && pos.y > 0){
    vec3 b = vec3(pos.x-1, SCALE*height(pos-ivec2(1,0)), pos.y);
    vec3 c = vec3(pos.x, SCALE*height(pos-ivec2(0,1)), pos.y-1);
    n += cross(c-p, b-p);
    k++;
  }

  if(pos.x > 0 && pos.y < dim.y - 1){
    vec3 b = vec3(pos.x-1, SCALE*height(pos-ivec2(1,0)), pos.y);
    vec3 c = vec3(pos.x, SCALE*height(pos+ivec2(0,1)), pos.y+1);
    n -= cross(c-p, b-p);
    k++;
  }

  if(pos.x < dim.x-1 && pos.y > 0){
    vec3 b = vec3(pos.x+1, SCALE*height(pos+ivec2(1,0)), pos.y);
    vec3 c = vec3(pos.x, SCALE*height(pos-ivec2(0,1)), pos.y-1);
    n -= cross(c-p, b-p);
    k++;
  }

  if(pos.x < dim.x-1 && pos.y < dim.y-1){
    vec3 b = vec3(pos.x+1, SCALE*height(pos+ivec2(1,0)), pos.y);
    vec3 c = vec3(pos.x, SCALE*height(pos+ivec2(0,1)), pos.y+1);
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
  if(dat[pos.x*dim.y+pos.y] == NULL) return 0;
  return dat[pos.x*dim.y+pos.y]->type;
}

double Layermap::height(ivec2 pos){
  if(dat[pos.x*dim.y+pos.y] == NULL) return 0.0;
  return (dat[pos.x*dim.y+pos.y]->floor + dat[pos.x*dim.y+pos.y]->size);
}

double Layermap::height(vec2 pos){

  double h = 0.0f;
  ivec2 p = floor(pos);
  vec2 w = fract(pos);

  h += (1.0-w.x)*(1.0-w.y)*height(p);
  h += (1.0-w.x)*w.y*height(p+ivec2(1,0));
  h += w.x*(1.0-w.y)*height(p+ivec2(0,1));
  h += w.x*w.y*height(p+ivec2(1,1));
  return h;

}
