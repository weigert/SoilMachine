/*
================================================================================
                      Wind Particle for Wind Erosion
================================================================================
*/

#include "particle.h"

using namespace glm;

struct WindParticle : public Particle {

  WindParticle(vec2 p):Particle(p){}

  WindParticle(vec2 p, Layermap& map):WindParticle(p){

    ipos = round(pos);
    surface = map.surface(ipos);
    param = pdict[surface];
    contains = param.transports;    //The Transporting Type

  }

  //Core Properties
  const vec3 pspeed = vec3(-2,0,1);
  vec3 speed = pspeed;
  double sediment = 0.0;  //Sediment Mass
  double height = 0.0;    //Particle Height
  double sheight = 0.0;   //Surface Height

  //Helper Properties
  ivec2 ipos;
  vec3 n;
  SurfType surface;
  SurfType contains;
  SurfParam param;

  const double gravity = 0.01;
  const double winddominance = 0.1;
  const double windfriction = 0.9;
  const double minsed = 0.0001;

  bool move(Layermap& map, Vertexpool<Vertex>& vertexpool){

    //Integer Position
    ipos = round(pos);
    n = map.normal(ipos);
    surface = map.surface(ipos);
    param = pdict[surface];

    //Surface Height, No-Clip Condition
    sheight = map.height(ipos);
    if(height < sheight){
      height = sheight;
    }

    //Movement Mechanics
    if(height > sheight)    //Flying Movement
      speed.y -= gravity;   //Gravity
    else                    //Contact Movement
      speed = mix(speed, cross(cross(speed,n),n), windfriction);

    speed = mix(speed, pspeed, winddominance);
    pos += vec2(speed.x, speed.z);
    height += speed.y;

    //Out-Of-Bounds
    if(!all(greaterThanEqual(pos, vec2(0))) ||
       !all(lessThan((ivec2)pos, map.dim-1)))
       return false;

    if(length(speed) < 0.01)
         return false;

    return true;

  }

  bool interact(Layermap& map, Vertexpool<Vertex>& vertexpool){

    //Non-Suspending
    if(pdict[contains].suspension == 0.0)
      return false;

    ivec2 npos = round(pos);

    //Surface Contact
    if(height <= map.height(pos)){

      //If this surface can conribute to this particle
      if(param.transports == contains){

        double force = length(speed)*(map.height(npos)-height)*(1.0f-sediment);

        double diff = map.remove(ipos, param.suspension*force);
        sediment += (param.suspension*force - diff);

        cascade(ipos, map, vertexpool, true);
        map.update(ipos, vertexpool);

      }

    }

    else {

      sediment -= pdict[contains].suspension*sediment;

      map.add(npos, map.pool.get(0.5f*pdict[contains].suspension*sediment, contains));
      map.add(ipos, map.pool.get(0.5f*pdict[contains].suspension*sediment, contains));

      cascade(ipos, map, vertexpool, true);
      map.update(ipos, vertexpool);

      cascade(npos, map, vertexpool, true);
      map.update(npos, vertexpool);

    }

    return true;

  }

};
