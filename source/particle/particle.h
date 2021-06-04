/*
================================================================================
        Particle Base Class for Geomorphological Transport Simulation
================================================================================
*/

#ifndef LAYEREDEROSION_PARTICLE
#define LAYEREDEROSION_PARTICLE

using namespace glm;

struct Particle {

  vec2 pos;
  vec2 speed = vec2(0);
  bool isalive = true;

  Particle(vec2 p):pos(p){}

  void move(Layermap& map);
  void interact(Layermap& map, Vertexpool<Vertex>& vertexpool);

};

/*
================================================================================
                  Water Particle for Hydraulic Erosion
================================================================================
*/

void cascade(vec2 pos, Layermap& map, Vertexpool<Vertex>& vertexpool){

  //Neighbor Positions (8-Way)
  const int nx[8] = {-1,-1,-1, 0, 0, 1, 1, 1};
  const int ny[8] = {-1, 0, 1,-1, 1,-1, 0, 1};

  glm::ivec2 ipos = pos;

  //Iterate over all Neighbors
  for(int m = 0; m < 8; m++){

    glm::ivec2 npos = ipos + ivec2(nx[m], ny[m]);

    if(npos.x >= map.dim.x-1 || npos.y >= map.dim.y-1) continue;
    if(npos.x <= 1 || npos.y <= 1) continue;

    //Full Height-Different Between Positions!
    float diff = (map.height(ipos) - map.height(npos));

    //The Maximum Difference Allowed between two Neighbors
    SurfType type = (diff > 0)?map.surface(ipos):map.surface(npos);
    SurfParam param = pdict[type];

    //The Amount of Excess Difference!
    float excess = abs(diff) - param.maxdiff;
    if(excess <= 0) continue; //No Excess

    //Actual Amount Transferred
    float transfer = excess / 2.0f;

    //Cap by Maximum Transferrable Amount
    if(diff > 0){
      transfer = (transfer < map.top(ipos)->size)?transfer:map.top(ipos)->size;
      map.remove(ipos, param.settling*transfer);
      map.add(npos, map.pool.get(param.settling*transfer, param.cascades));
    }
    else{
      transfer = (transfer < map.top(npos)->size)?transfer:map.top(npos)->size;
      map.remove(npos, param.settling*transfer);
      map.add(ipos, map.pool.get(param.settling*transfer, param.cascades));
    }

  }

}


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

#endif
