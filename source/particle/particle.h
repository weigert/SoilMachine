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

  bool move(Layermap& map);
  bool interact(Layermap& map, Vertexpool<Vertex>& vertexpool);

  //This is applied to multiple types of erosion, so I put it in here!
  static void cascade(vec2 pos, Layermap& map, Vertexpool<Vertex>& vertexpool, int transferloop = 0){

    ivec2 ipos = round(pos);

    // All Possible Neighbors

    vector<ivec2> n = {
      ivec2(-1, -1),
      ivec2(-1,  0),
      ivec2(-1,  1),
      ivec2( 0, -1),
      ivec2( 0,  1),
      ivec2( 1, -1),
      ivec2( 1,  0),
      ivec2( 1,  1)
    };

    //No Out-Of-Bounds

    vector<ivec2> sn;
    for(auto& nn: n){
      ivec2 npos = ipos + nn;
      if(npos.x >= map.dim.x || npos.y >= map.dim.y
         || npos.x < 0 || npos.y < 0) continue;
      sn.push_back(npos);
    }

    // Sort by Steepness
    sort(sn.begin(), sn.end(), [&](const ivec2& a, const ivec2& b){
      return map.height(a) < map.height(b);
    });

    for(auto& npos: sn){

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
        map.update(npos, vertexpool);
        map.update(ipos, vertexpool);
      }
      else{
        if(transfer > map.top(npos)->size)
          transfer = map.top(npos)->size;
        double diff = map.remove(npos, transfer);
        if(diff != 0) recascade = true;
        map.add(ipos, map.pool.get(transfer, param.cascades));
        map.update(npos, vertexpool);
        map.update(ipos, vertexpool);
      }

      if(recascade && transferloop > 0)
        cascade(npos, map, vertexpool, --transferloop);

    }

  }

};

#endif
