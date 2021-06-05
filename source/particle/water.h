/*
================================================================================
                  Water Particle for Hydraulic Erosion
================================================================================
*/

#include "particle.h"

struct WaterParticle : public Particle {

  WaterParticle(vec2 p):Particle(p){}

  WaterParticle(vec2 p, Layermap& map):WaterParticle(p){

    ipos = round(pos);
    surface = map.surface(ipos);
    param = pdict[surface];
    contains = param.transports;    //The Transporting Type

  }

  //Core Properties
  float volume = 1.0;   //This will vary in time
  float sediment = 0.0; //Fraction of Volume that is Sediment!

  const float minvol = 0.005;
  const float evaprate = 0.01;

  //Helper Properties
  ivec2 ipos;
  vec3 n;
  SurfParam param;
  SurfType surface;
  SurfType contains;

  bool move(Layermap& map, Vertexpool<Vertex>& vertexpool){

    ipos = round(pos);                //Position
    n = map.normal(ipos);             //Surface Normal Vector
    surface = map.surface(ipos);      //Surface Composition
    param = pdict[surface];           //Surface Composition Parameter Set

    if(length(vec2(n.x, n.z)/(volume)*param.friction) < 0.005)   //No Motion
      return false;

    //Motion Low
    speed = mix(speed, vec2(n.x, n.z)/(volume), param.friction);
    speed = sqrt(2.0f)*normalize(speed);
    pos   += speed;

    //Out-of-Bounds
    if(!glm::all(glm::greaterThanEqual(pos, vec2(0))) ||
       !glm::all(glm::lessThan(pos, (vec2)map.dim-1.0f)))
       return false;

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
    if(dist::uniform() < pdict[contains].erosionrate)
      contains = pdict[contains].erodes;

    //Add Sediment to Map
    if(cdiff < 0)
      map.add(ipos, map.pool.get(-volume*param.equrate*cdiff, contains));

    //Remove Sediment from Map
    if(cdiff > 0){
      double diff = map.remove(ipos, volume*param.equrate*cdiff);
      while(diff > 0.0) diff = map.remove(ipos, diff);
    }

    //Execute Sediment Cascade at Location
    cascade(pos, map, vertexpool);

    //Update Map, Particle
    map.update(ipos, vertexpool);
    sediment /= (1.0-evaprate);
    volume *= (1.0-evaprate);
    return (volume > minvol);

  }

};
