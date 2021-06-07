/*
================================================================================
                  Water Particle for Hydraulic Erosion
================================================================================
*/

#include "particle.h"

struct WaterParticle : public Particle {

  WaterParticle(vec2 p):Particle(p){}

  WaterParticle(vec2 p, Layermap& map):WaterParticle(p){

    ipos = round(pos);
    surface = map.surface(ipos);
    param = pdict[surface];
    contains = param.transports;    //The Transporting Type

  }

  //Core Properties
  float volume = 1.0;   //This will vary in time
  float sediment = 0.0; //Fraction of Volume that is Sediment!

  const float minvol = 0.005;
  const float evaprate = 0.01;

  //Helper Properties
  ivec2 ipos;
  int spill = 5;
  vec3 n;
  SurfParam param;
  SurfType surface;
  SurfType contains;

  bool move(Layermap& map, Vertexpool<Vertex>& vertexpool){

    ipos = round(pos);                //Position
    n = map.normal(ipos);             //Surface Normal Vector
    surface = map.surface(ipos);      //Surface Composition
    param = pdict[surface];           //Surface Composition

    if(surface == WATER)
      return false;

    if(length(vec2(n.x, n.z)*param.friction) < 1E-3)   //No Motion
      return false;

    //Motion Low
    speed = mix(speed, vec2(n.x, n.z), param.friction);
    speed = sqrt(2.0f)*normalize(speed);
    pos   += speed;

    //Out-of-Bounds
    if(!glm::all(glm::greaterThanEqual(pos, vec2(0))) ||
       !glm::all(glm::lessThan(pos, (vec2)map.dim-1.0f))){
         volume = 0.0;
         return false;
       }

    return true;

  }

  bool interact(Layermap& map, Vertexpool<Vertex>& vertexpool){

    //Equilibrium Sediment Transport Amount
    float c_eq = param.solubility*length(vec2(n.x,n.z))*(map.height(ipos)-map.height(pos));
    if(c_eq < 0.0) c_eq = 0.0;

    //Execute Transport to Particle
    float cdiff = c_eq - sediment;
    sediment += param.equrate*cdiff;

    //Erode Sediment IN Particle
    //if(dist::uniform() < pdict[contains].erosionrate)
    if((float)(rand()%10000)/10000.0f < pdict[contains].erosionrate)
      contains = pdict[contains].erodes;

    //Add Sediment to Map
    if(cdiff < 0)
      map.add(ipos, map.pool.get(-param.equrate*cdiff, contains));

    //Remove Sediment from Map
    if(cdiff > 0){
      double diff = map.remove(ipos, param.equrate*cdiff);
      while(diff > 0.0) diff = map.remove(ipos, diff);
    }

    //Execute Sediment Cascade at Location
    cascade(pos, map, vertexpool);

    //Update Map, Particle
    map.update(ipos, vertexpool);
    sediment /= (1.0-evaprate);
    volume *= (1.0-evaprate);
    return (volume > minvol);

  }

  bool flood(Layermap& map, Vertexpool<Vertex>& vertexpool){

    if(volume < minvol || spill-- <= 0)
      return false;

    ipos = pos;                         //Position
    double plane = map.height(ipos);    //Height of Map at Position
    double initialplane = plane;        //Store Initial Plane

    vector<ivec2> set;                  //Flood-Set
    int fail = 15;                      //Number of Tries
    const double volumeFactor = 100.0;   //"Water Deposition Rate"

    //Can be declared here!
    ivec2 drain = ipos;
    bool drainfound = false;

    vector<ivec2> oldboundary;             //Flood-Set
    vector<ivec2> newboundary;
    oldboundary.push_back(ipos);

    //set.clear();
    bool tried[map.dim.x*map.dim.y] = {false};

    while(volume > minvol && fail){     //Try to Flood with Volume

      const function<void(ivec2)> fill = [&](ivec2 ppos){

        //Out of Bounds
        if(!all(greaterThanEqual(ppos, ivec2(0))) ||
          !all(lessThan(ppos, map.dim)))
          return;

        //Position has been tried
        if(tried[ppos.x*map.dim.y+ppos.y])
          return;

        //Wall / Boundary
        if(plane < map.height(ppos)){
          newboundary.push_back(ppos);
          return;
        }

        //Position is now tried!
        tried[ppos.x*map.dim.y+ppos.y] = true;

        //Drainage Point
        if(initialplane > map.height(ppos)){

          //No Drain yet
          if(!drainfound)
            drain = ppos;

          //Lower Drain
          else if( map.height(drain) < map.height(ppos) )
            drain = ppos;

          drainfound = true;
          return;

        }

        set.push_back(ppos);        //Part of the Pool
        fill(ppos+ivec2( 0, 1));    //Fill Neighbors
        fill(ppos+ivec2( 0,-1));    //Fill Neighbors
        fill(ppos+ivec2( 1, 0));    //Fill Neighbors
        fill(ppos+ivec2(-1, 0));    //Fill Neighbors
        fill(ppos+ivec2( 1, 1));    //Fill Neighbors
        fill(ppos+ivec2( 1,-1));    //Fill Neighbors
        fill(ppos+ivec2(-1, 1));    //Fill Neighbors
        fill(ppos+ivec2(-1,-1));    //Fill Neighbors

      };

      newboundary.clear();
      for(auto& b: oldboundary){
        fill(b);
      }
      oldboundary = newboundary;

      //boundary.clear();

      //Drainage Point -> Exit Loop!
      if(drainfound){

        pos = drain;

        double drainage = 0.01;
        plane = (1.0-drainage)*initialplane + drainage*(map.height(drain));

        //Compute the New Height
        for(auto& s: set){

          if(map.surface(s) != WATER)
            continue;

          double h = (map.height(s)-plane);
          double diff = map.remove(s, h);
          map.update(s, vertexpool);

        }

        sediment = 0.0f;
        return true;

      }

      //Get Volume under Plane
      double tVol = 0.0;
      for(auto& s: set)
        tVol += volumeFactor*(plane - map.height(s));

      //We can partially fill this volume
      if(tVol <= volume && initialplane < plane){

        //Raise water level to plane height
        for(auto& s: set){
          map.add(s, map.pool.get(plane-map.height(s), WATER));
          map.update(s, vertexpool);
        }

        //Adjust Drop Volume
        volume -= tVol;
        tVol = 0.0;

      }

      //Plane was too high.
      else
        fail--;

      //Adjust Planes
      initialplane = (plane > initialplane)?plane:initialplane;
      plane += 0.5*(volume-tVol)/(double)set.size()/volumeFactor;

    }

    if(volume < minvol || !fail)
      return false;

    return true;

  }

};
