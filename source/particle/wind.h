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
  const vec3 pspeed = normalize(vec3(-1,0,1));
  vec3 speed = pspeed;
  double height = 0.0;
  double sediment = 0.0; //Sediment Mass


  //Helper Properties
  ivec2 ipos;

  //Parameters
  const double gravity = 0.01;
  const double suspension = 0.0001;  //Affects transport rate

  bool move(Layermap& map, Vertexpool<Vertex>& vertexpool){

    //Integer Position
    ipos = round(pos);

    //Surface Height
    double sheight = map.height(ipos);
    if(height < sheight) height = sheight;

    vec3 n = map.normal(ipos);

    //Movement Mechanics

    if(height > sheight)    //Flying Movement
      speed.y -= gravity;   //Gravity
    else{                   //Contact Movement
      speed = mix(speed, pspeed, 0.1f);
      speed = mix(speed, cross(cross(normalize(speed),n),n), 0.2f);
      speed = sqrt(2.0f)*normalize(speed);
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

/*
      //Abrasion
      if(s[ind] <= 0){

        s[ind] = 0;
        h[ind] -= dt*abrasion*force*sediment;
        s[ind] += dt*abrasion*force*sediment;

      }
      */

      //Remove Sediment Amount
      sediment += suspension*force;

      map.remove(ipos, suspension*force);

    //  map.remove(npos, 0.5f*suspension*force);
      cascade(ipos, map, vertexpool);
    //  cascade(npos, map, vertexpool);
      map.update(ipos, vertexpool);
    //  map.update(npos, vertexpool);

    }

    //Flying Particle
    else{

      sediment -= suspension*sediment;

      map.add(npos, map.pool.get(0.5*suspension*sediment, SAND));
      map.add(ipos, map.pool.get(0.5*suspension*sediment, SAND));
      cascade(npos, map, vertexpool);
      cascade(ipos, map, vertexpool);
      map.update(npos, vertexpool);
      map.update(ipos, vertexpool);

    }

    return (sediment >= 1E-6);

  }

};
