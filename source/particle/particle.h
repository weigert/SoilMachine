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

  bool move(Layermap& map);
  bool interact(Layermap& map, Vertexpool<Vertex>& vertexpool);

  //This is applied to multiple types of erosion, so I put it in here!
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

};

#endif
