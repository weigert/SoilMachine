/*
================================================================================
                  Water Particle for Hydraulic Erosion
================================================================================
*/

#include "particle.h"

struct WaterParticle : public Particle {

  WaterParticle(vec2 p):Particle(p){}

  //Core Properties
  float volume = 1.0;   //This will vary in time
  float sediment = 0.0; //Fraction of Volume that is Sediment!
  float density = 1.0f; //Density of Internal Sediment

  bool init = true;

  //Helper Properties
  ivec2 ipos;
  vec3 n;
  SurfType type;
  SurfParam param;

  const float minvol = 0.005;
  const float evaprate = 0.01;

  bool move(Layermap& map, Vertexpool<Vertex>& vertexpool){

    ipos = round(pos);
    n = map.normal(ipos);

    //Surface Parameters!
    type = map.surface(ipos);
    param = pdict[type];

    if(length(vec2(n.x, n.z)/(volume*density)*param.friction) < 0.005)   //No Motion
      return false;

    speed = mix(speed, vec2(n.x, n.z)/(volume*density), param.friction);
    speed = sqrt(2.0f)*normalize(speed);
    pos   += speed;

    if(!glm::all(glm::greaterThanEqual(pos, vec2(0))) ||  //Out-of-Bounds
       !glm::all(glm::lessThan(pos, (vec2)map.dim-1.0f)))
       return false;

    return true;

  }

  bool interact(Layermap& map, Vertexpool<Vertex>& vertexpool){

    //Edge Case
    if(type == AIR){
      map.add(ipos, map.pool.get(0.0001f, ROCK));
      map.update(ipos, vertexpool);
      return false;
    }

    if(init){
      density = param.density;
      init = false;
    }

    float c_eq = param.solubility*length(vec2(n.x,n.z)/(volume*density))*volume*(map.height(ipos)-map.height(pos));
    if(c_eq < 0.0) c_eq = 0.0;

    float cdiff = c_eq - sediment;

    if(cdiff < 0){
      density = (sediment*density - param.erosiveness*param.equrate*cdiff*param.density);
      density /= (sediment - param.equrate*cdiff);
    }

    sediment += param.equrate*cdiff;

    //Regular Case

    if(cdiff < 0){
      if(density <= pdict[param.erodes].density)
        map.add(ipos, map.pool.get(-volume*density*param.equrate*cdiff, param.erodes));
      else
        map.add(ipos, map.pool.get(-volume*density*param.equrate*cdiff, type));
    }
    if(cdiff > 0){
      double diff = map.remove(ipos, volume*density*param.equrate*cdiff);
      while(diff > 0.0) diff = map.remove(ipos, diff);
    }

    cascade(pos, map, vertexpool);
    map.update(ipos, vertexpool);

    //Change Water Particle Mass
    sediment /= (1.0-evaprate);
    volume *= (1.0-evaprate);
    return (volume > minvol);

  }

};
