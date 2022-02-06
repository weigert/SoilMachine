/*
================================================================================
                  Water Particle for Hydraulic Erosion
================================================================================
*/

#include "particle.h"

#include <unordered_map>
#include <unordered_set>

struct Pool {         //Water Level Acceleration Structure

  struct poolhash {
    size_t operator()(const ivec2& k) const {
      return std::hash<int>()(k.x) ^ std::hash<int>()(k.y);
    }

    bool operator()(const ivec2& a, const ivec2& b) const {
      return a.x == b.x && a.y == b.y;
    }
  };

  double plane;       //Height of the Plane

  unordered_set<ivec2, poolhash, poolhash> set;
  unordered_map<ivec2, double, poolhash, poolhash> boundary;

  pair<ivec2, double> minbound;
  pair<ivec2, double> drain;
  bool drained = false;

  Pool(Layermap& map, ivec2 pos){
    plane = map.height(pos);
    boundary[pos] = plane;
    minbound = pair<ivec2, double>(pos, plane);
  }

};

struct WaterParticle : public Particle {

  WaterParticle(Layermap& map){

    pos = vec2(rand()%map.dim.x, rand()%map.dim.y);
    ipos = round(pos);
    surface = map.surface(ipos);
    param = soils[surface];
    contains = param.transports;    //The Transporting Type

  }

  //Core Properties
  float volume = 1.0;   //This will vary in time
  float sediment = 0.0; //Fraction of Volume that is Sediment!

  const float minvol = 0.01;
  float evaprate = 0.001;

  const double volumeFactor = 0.5;    //"Water Deposition Rate"
  int spill = 3;

  //Helper Properties
  ivec2 ipos;
  vec3 n;
  SurfParam param;
  SurfType surface;
  SurfType contains;

  static float* frequency;
  static float* track;

  void updatefrequency(Layermap& map, ivec2 ipos){
    int ind = ipos.y*map.dim.x+ipos.x;
    track[ind] += volume;
  }

  static void resetfrequency(Layermap& map){
    for(int i = 0; i < map.dim.x*map.dim.y; i++)
      track[i] = 0.0f;
  }

  static void mapfrequency(Layermap& map){
    const float lrate = 0.05f;
    const float K = 15.0f;
    for(int i = 0; i < map.dim.x*map.dim.y; i++)
      frequency[i] = (1.0f-lrate)*frequency[i] + lrate*K*track[i]/(1.0f + K*track[i]);;
  }

  bool move(Layermap& map, Vertexpool<Vertex>& vertexpool){

    ipos = round(pos);                //Position
    n = map.normal(ipos);             //Surface Normal Vector
    surface = map.surface(ipos);      //Surface Composition
    param = soils[surface];           //Surface Composition
    updatefrequency(map, ipos);

    //Modify Parameters Based on Frequency
    param.friction = param.friction*(1.0f-frequency[ipos.y*map.dim.x+ipos.x]);
    evaprate = 0.005f*(1.0f-0.2f*frequency[ipos.y*map.dim.x+ipos.x]);

  //  if(surface == soilmap["Water"] || surface == soilmap["Air"])
  //    return false;

    if(length(vec2(n.x, n.z)*param.friction) < 1E-6)   //No Motion
      return false;

    //Motion Low
    speed = mix(vec2(n.x, n.z), speed, param.friction);
    speed = sqrt(2.0f)*normalize(speed);
    pos   += speed;

    //Out-of-Bounds
    if(!glm::all(glm::greaterThanEqual(pos, vec2(0))) ||
       !glm::all(glm::lessThan(pos, (vec2)map.dim-1.0f))){
         volume = 0.0;
         return false;
       }

    return true;

  }

  bool interact(Layermap& map, Vertexpool<Vertex>& vertexpool){

    //Equilibrium Sediment Transport Amount
    float c_eq = param.solubility*(map.height(ipos)-map.height(pos));
    if(c_eq < 0.0) c_eq = 0.0;

    //Execute Transport to Particle
    float cdiff = c_eq - sediment;
    sediment += param.equrate*cdiff;

    //Erode Sediment IN Particle
    if((float)(soils[contains].erosionrate) < frequency[ipos.y*map.dim.x+ipos.x])
      contains = soils[contains].erodes;

    //Add Sediment to Map
    if(cdiff < 0)
      map.add(ipos, map.pool.get(-param.equrate*cdiff, contains));

    //Remove Sediment from Map
    if(cdiff > 0){
      double diff = map.remove(ipos, param.equrate*cdiff);
      while(diff > 0.0) diff = map.remove(ipos, diff);
    }

    //Particle Cascade: Thermal Erosion!
    Particle::cascade(pos, map, vertexpool, 0);

    //Update Map, Particle
    map.update(ipos, vertexpool);
    sediment /= (1.0-evaprate);
    volume *= (1.0-evaprate);
    return (volume > minvol);

  }

  bool flood(Layermap& map, Vertexpool<Vertex>& vertexpool){

    if(volume < minvol || spill-- <= 0)
      return false;

    ipos = pos;                         //Position

    map.add(ipos, map.pool.get(volume*0.001, soilmap["Water"]));


    //We can attempt to do a cascade at this position, but basically because
    //we are water we want to either spawn a new particle at the next location
    //or we want to cascade onto water.


    Particle::cascade(ipos, map, vertexpool);
    map.update(ipos, vertexpool);

    return false;

  }

};

float* WaterParticle::frequency = new float[SIZEX*SIZEY]{0.0f};
float* WaterParticle::track = new float[SIZEX*SIZEY]{0.0f};
