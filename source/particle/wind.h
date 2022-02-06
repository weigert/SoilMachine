/*
================================================================================
                      Wind Particle for Wind Erosion
================================================================================
*/

#include "particle.h"

using namespace glm;

struct WindParticle : public Particle {

  WindParticle(Layermap& map){

    pos = vec2(rand()%map.dim.x, rand()%map.dim.y);

    ipos = round(pos);
    surface = map.surface(ipos);
    param = soils[surface];
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

  const double gravity = 0.1;
  const double winddominance = 0.2;
  const double windfriction = 0.8;
  const double minsed = 0.0001;


  static float* frequency;
  void updatefrequency(Layermap& map, ivec2 ipos){
    int ind = ipos.y*map.dim.x+ipos.x;
    frequency[ind] = 0.5*frequency[ind] + 0.5f;
  }

  bool move(Layermap& map, Vertexpool<Vertex>& vertexpool){

    if(soils[contains].suspension == 0.0)
      return false;

    //Integer Position
    ipos = round(pos);
    n = map.normal(ipos);
    surface = map.surface(ipos);
    param = soils[surface];
    updatefrequency(map, ipos);

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

  //  if(param.abrasion == 0.0)
  //    return true;

    ivec2 npos = round(pos);

    //Surface Contact
    if(height <= map.height(pos)){

      //If this surface can conribute to this particle
      if(param.transports == contains){

        double force = length(speed)*(map.height(npos)-height)*(1.0f-sediment);

        double diff = map.remove(ipos, param.suspension*force);
        sediment += (param.suspension*force - diff);

        cascade(ipos, map, vertexpool, 0);
        map.update(ipos, vertexpool);

      }

    }

    else if(param.suspension > 0.0){

      sediment -= soils[contains].suspension*sediment;

      map.add(npos, map.pool.get(0.5f*soils[contains].suspension*sediment, contains));
      map.add(ipos, map.pool.get(0.5f*soils[contains].suspension*sediment, contains));

      cascade(ipos, map, vertexpool, 0);
      map.update(ipos, vertexpool);

      cascade(npos, map, vertexpool, 0);
      map.update(npos, vertexpool);

    }

    return true;

  }

};

float* WindParticle::frequency =  new float[SIZEX*SIZEY]{0.0f};
