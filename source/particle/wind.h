/*
================================================================================
                      Wind Particle for Wind Erosion
================================================================================
*/

#include "particle.h"

using namespace glm;

struct WindParticle : public Particle {

  WindParticle(vec2 p):Particle(p){}

  //Core Properties
  const vec3 pspeed = normalize(vec3(1,0,-1));
  vec3 speed = pspeed;
  double height = 0.0;
  double sediment = 0.0; //Sediment Mass

  int ncycles = 5000;

  //Helper Properties
  ivec2 ipos;
  SurfParam param;

  //Parameters
  const double gravity = 0.05;

  bool move(Layermap& map, Vertexpool<Vertex>& vertexpool){

    ncycles--;
    if(ncycles <= 0) return false;

    //Integer Position
    ipos = round(pos);
    param = pdict[map.surface(ipos)];

    //Surface Height
    double sheight = map.height(ipos);
    if(height < sheight) height = sheight;

    vec3 n = map.normal(ipos);

    //Movement Mechanics

    if(height > sheight)    //Flying Movement
      speed.y -= gravity;   //Gravity
    else{                   //Contact Movement
      speed = mix(speed, pspeed, 0.1f);
      speed = mix(speed, cross(cross(speed,n),n), 0.2f);
      speed = normalize(speed);
    }

    pos += vec2(speed.x, speed.z);
    height += speed.y;

    //Out-Of-Bounds
    if(!all(greaterThanEqual(pos, vec2(0))) ||
       !all(lessThan((ivec2)pos, map.dim-1)))
       return false;

    return true;

  }

  bool interact(Layermap& map, Vertexpool<Vertex>& vertexpool){

    ivec2 npos = round(pos);

    //Surface Contact
    if(height <= map.height(pos)){

      double force = (map.height(npos)-height);

      //Abrasion
    //  if(param.abrasion > 0.0){
    //    map.remove(ipos, param.abrasion*force*sediment);
    //    map.add(ipos, map.pool.get(param.abrasion*force*sediment, param.abrades));
    //    param = pdict[map.surface(npos)];
    //  }
    //  else{

        //Remove Sediment Amount
        sediment += param.suspension*force;
        map.remove(ipos, param.suspension*force);

        if(param.settling > 0.0)
          cascade(ipos, map, vertexpool);

    //  }

      map.update(ipos, vertexpool);

    }

    //Flying Particle
    else{

      sediment -= param.suspension*sediment;

      map.add(npos, map.pool.get(0.5f*param.suspension*sediment, param.abrades));
      map.add(ipos, map.pool.get(0.5f*param.suspension*sediment, param.abrades));

      if(param.settling > 0.0){
        cascade(npos, map, vertexpool);
        cascade(ipos, map, vertexpool);
      }
      map.update(npos, vertexpool);
      map.update(ipos, vertexpool);

    }

    return (sediment >= 1E-5);

  }

};
