/*
================================================================================
        Particle Base Class for Geomorphological Transport Simulation
================================================================================
*/

#ifndef LAYEREDEROSION_PARTICLE
#define LAYEREDEROSION_PARTICLE

#include "../include/distribution.h"

using namespace glm;

struct Particle {

  vec2 pos;
  vec2 speed = vec2(0);
  bool isalive = true;

  Particle(vec2 p):pos(p){}

  bool move(Layermap& map);
  bool interact(Layermap& map, Vertexpool<Vertex>& vertexpool);

  //This is applied to multiple types of erosion, so I put it in here!
  static void cascade(vec2 pos, Layermap& map, Vertexpool<Vertex>& vertexpool, int transferloop = 0){

    ivec2 ipos = round(pos);

    //Neighbor Positions (8-Way)
    const int nx[8] = {-1,-1,-1, 0, 0, 1, 1, 1};
    const int ny[8] = {-1, 0, 1,-1, 1,-1, 0, 1};

    //Iterate over all Neighbors
    for(int m = 0; m < 8; m++){

      ivec2 npos = ipos + ivec2(nx[m], ny[m]);
      if(npos.x >= map.dim.x || npos.y >= map.dim.y
         || npos.x < 0 || npos.y < 0) continue;

      //Full Height-Different Between Positions!
      float diff = (map.height(ipos) - map.height(npos));
      if(diff == 0)   //No Height Difference
        continue;

      //The Maximum Difference Allowed between two Neighbors
      SurfType type = (diff > 0)?map.surface(ipos):map.surface(npos);
      SurfParam param = soils[type];

      //The Amount of Excess Difference!
      float excess = abs(diff) - param.maxdiff;
      if(excess <= 0)  //No Excess
        continue;

      //Actual Amount Transferred
      float transfer = param.settling * excess / 2.0f;

      bool recascade = false;

      //Cap by Maximum Transferrable Amount
      if(diff > 0){
        if(transfer > map.top(ipos)->size)
          transfer = map.top(ipos)->size;
        if(transfer > 0) recascade = true;
        double diff = map.remove(ipos, transfer);
        if(diff != 0) recascade = true;
        map.add(npos, map.pool.get(transfer, param.cascades));
      }
      else{
        if(transfer > map.top(npos)->size)
          transfer = map.top(npos)->size;
        double diff = map.remove(npos, transfer);
        if(diff != 0) recascade = true;
        map.add(ipos, map.pool.get(transfer, param.cascades));
      }

      if(recascade && transferloop > 0)
        cascade(npos, map, vertexpool, --transferloop);

    }

    map.update(ipos, vertexpool);

  }

};

#endif
