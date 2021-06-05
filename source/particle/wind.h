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
  const vec3 pspeed = vec3(-2,0,2);
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

  const double gravity = 0.05;
  const double winddominance = 0.2;
  const double windfriction = 0.2;
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

    if(contains == AIR)
      return false;

    if(param.suspension == 0.0) //Soil-Type Does Not Interact!
      return false;

    ivec2 npos = round(pos);

    //Surface Contact
    if(height <= map.height(pos) && param.suspension > 0.0){

      double force = length(speed)*(map.height(npos)-height);

      double diff = map.remove(ipos, param.suspension*force);
      sediment += (param.suspension*force - diff);

      cascade(ipos, map, vertexpool, true);
      map.update(ipos, vertexpool);

    }

    if(param.suspension*sediment > minsed){

      sediment -= param.suspension*sediment;

      map.add(npos, map.pool.get(0.5f*param.suspension*sediment, contains));
      map.add(ipos, map.pool.get(0.5f*param.suspension*sediment, contains));

      cascade(npos, map, vertexpool, true);
      map.update(npos, vertexpool);

      cascade(ipos, map, vertexpool, true);
      map.update(ipos, vertexpool);

    }

    return true;

  }

};

/*
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
      double diff = (param.suspension*force-sediment);
      sediment += diff;
      map.remove(ipos, diff);
      cascade(ipos, map, vertexpool, true);
      map.update(ipos, vertexpool);

    }

    //Flying Particle
    else{

      sediment -= param.suspension*sediment;

      map.add(npos, map.pool.get(0.5f*param.suspension*sediment, param.abrades));
      map.add(ipos, map.pool.get(0.5f*param.suspension*sediment, param.abrades));
      cascade(npos, map, vertexpool, true);
      cascade(ipos, map, vertexpool, true);
      map.update(npos, vertexpool);
      map.update(ipos, vertexpool);

      if(sediment < 1E-6) return false;

    }

    return true;

  }

};
*/
