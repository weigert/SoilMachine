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

  WaterParticle(vec2 p):Particle(p){}

  WaterParticle(vec2 p, Layermap& map):WaterParticle(p){

    ipos = round(pos);
    surface = map.surface(ipos);
    param = soils[surface];
    contains = param.transports;    //The Transporting Type

  }

  //Core Properties
  float volume = 1.0;   //This will vary in time
  float sediment = 0.0; //Fraction of Volume that is Sediment!

  const float minvol = 0.005;
  float evaprate = 0.01;

  const double volumeFactor = 0.5;    //"Water Deposition Rate"

  //Helper Properties
  ivec2 ipos;
  int spill = 3;
  vec3 n;
  SurfParam param;
  SurfType surface;
  SurfType contains;

  static float* frequency;

  void updatefrequency(Layermap& map, ivec2 ipos){
    int ind = ipos.y*map.dim.x+ipos.x;
    frequency[ind] = 0.95*frequency[ind] + 0.05f*volume;
  }

  bool move(Layermap& map, Vertexpool<Vertex>& vertexpool){

    ipos = round(pos);                //Position
    n = map.normal(ipos);             //Surface Normal Vector
    surface = map.surface(ipos);      //Surface Composition
    param = soils[surface];           //Surface Composition
    updatefrequency(map, ipos);

    //Modify Parameters Based on Frequency
  //  param.friction = param.friction + (1.0f-param.friction)*frequency[ipos.y*map.dim.x+ipos.x];

    if(surface == soilmap["Water"])
      return false;

    if(length(vec2(n.x, n.z)*(1.0f-param.friction)) < 1E-5)   //No Motion
      return false;

    //Motion Low
    speed = mix(speed, vec2(n.x, n.z), param.friction);
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
    float c_eq = param.solubility*length(vec2(n.x,n.z))*(map.height(ipos)-map.height(pos));
    if(c_eq < 0.0) c_eq = 0.0;

    //Execute Transport to Particle
    float cdiff = c_eq - sediment;
    sediment += param.equrate*cdiff;

    //Erode Sediment IN Particle
    if((float)(rand()%10000)/10000.0f < soils[contains].erosionrate)
      contains = soils[contains].erodes;

    //Add Sediment to Map
    if(cdiff < 0)
      map.add(ipos, map.pool.get(-param.equrate*cdiff, contains));

    //Remove Sediment from Map
    if(cdiff > 0){
      double diff = map.remove(ipos, param.equrate*cdiff);
      while(diff > 0.0) diff = map.remove(ipos, diff);
    }

    cascade(pos, map, vertexpool, 0);

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

    Pool pool(map, ipos);
    bool tried[map.dim.x*map.dim.y] = {false};

    while(volume > minvol){     //Try to Flood with Volume

      const function<void(const ivec2)> fill = [&](const ivec2 ppos){

        //Out of Bounds
        if(!all(greaterThanEqual(ppos, ivec2(0))) ||
          !all(lessThan(ppos, map.dim)))
          return;

        //Position has been tried
        if(tried[ppos.x*map.dim.y+ppos.y])
          return;

        //Store Height at this Position
        double height = map.height(ppos);

        //Wall / Boundary: Add to Boundary, Not in Set
        if(pool.plane < height){
          pool.boundary[ppos] = height;
          return;
        }

        //Drainage Point
        if(pool.plane > height){

          //No Drain yet
          if(!pool.drained)
            pool.drain = pair<ivec2, double>(ppos, height);

          //Lower Drain
          else if( height < pool.drain.second )
            pool.drain = pair<ivec2, double>(ppos, height);

          pool.drained = true;
          return;

        }

        tried[ppos.x*map.dim.y+ppos.y] = true;

        pool.set.insert(ppos);      //Part of the Pool
        fill(ppos+ivec2( 0, 1));    //Fill Neighbors
        fill(ppos+ivec2( 0,-1));    //Fill Neighbors
        fill(ppos+ivec2( 1, 0));    //Fill Neighbors
        fill(ppos+ivec2(-1, 0));    //Fill Neighbors
        fill(ppos+ivec2( 1, 1));    //Fill Neighbors
        fill(ppos+ivec2( 1,-1));    //Fill Neighbors
        fill(ppos+ivec2(-1, 1));    //Fill Neighbors
        fill(ppos+ivec2(-1,-1));    //Fill Neighbors

      };

      fill(pool.minbound.first);                    //Only Fill At Boundary

      if(pool.set.empty())
        break;

      //Drainage Point -> Exit Loop!
      if(pool.drained){

        pos = pool.drain.first;
        pool.plane = pool.minbound.second;

        //Compute the New Height
        for(auto& s: pool.set){

          if(map.surface(s) != soilmap["Water"])
            continue;

          double h = (map.height(s)-pool.plane);
          double diff = map.remove(s, h);
          volume += (h-diff)/volumeFactor;
          map.update(s, vertexpool);

        }

        return true;  //Repeat Flood Step

      }

      pool.boundary.erase(pool.minbound.first);     //Remove From Boundary Set

      if(pool.boundary.empty())
        break;

      pool.minbound = (*pool.boundary.begin()); //Lowest Boundary Element
      for(auto& b : pool.boundary)
      if(b.second < pool.minbound.second)
        pool.minbound = b;


      //We can set the plane to the height of the min boundary, or below
      double vheight = volume/(double)pool.set.size()*volumeFactor;
      if(pool.plane + vheight >= pool.minbound.second){
        volume -= (pool.minbound.second - pool.plane)*pool.set.size()/volumeFactor;
        pool.plane = pool.minbound.second;
      }
      else{
        volume = 0.0f;
        pool.plane += vheight;
      }

      //Raise water level to plane height
      for(auto& s: pool.set)
        map.add(s, map.pool.get(pool.plane-map.height(s), soilmap["Water"]));

    }

    for(auto& s: pool.set)
      map.update(s, vertexpool);

    return false;

  }

};

float* WaterParticle::frequency = new float[SIZEX*SIZEY]{0.0f};
