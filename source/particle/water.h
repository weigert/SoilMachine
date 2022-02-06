/*
================================================================================
                  Water Particle for Hydraulic Erosion
================================================================================
*/

#include "particle.h"

struct WaterParticle : public Particle {

  WaterParticle(Layermap& map){

    pos = vec2(rand()%map.dim.x, rand()%map.dim.y);
    ipos = round(pos);
    surface = map.surface(ipos);
    param = soils[surface];
    contains = param.transports;    //The Transporting Type

  }

  //Core Properties
  float volume = 1.0;   //This will vary in time
  float sediment = 0.0; //Fraction of Volume that is Sediment!

  const float minvol = 0.01;
  float evaprate = 0.001;

  const double volumeFactor = 0.001;    //"Water Deposition Rate"
  int spill = 3;

  //Helper Properties
  ivec2 ipos;
  vec3 n;
  SurfParam param;
  SurfType surface;
  SurfType contains;

  static float* frequency;
  static float* track;

  void updatefrequency(Layermap& map, ivec2 ipos){
    int ind = ipos.y*map.dim.x+ipos.x;
    track[ind] += volume;
  }

  static void resetfrequency(Layermap& map){
    for(int i = 0; i < map.dim.x*map.dim.y; i++)
      track[i] = 0.0f;
  }

  static void mapfrequency(Layermap& map){
    const float lrate = 0.05f;
    const float K = 15.0f;
    for(int i = 0; i < map.dim.x*map.dim.y; i++)
      frequency[i] = (1.0f-lrate)*frequency[i] + lrate*K*track[i]/(1.0f + K*track[i]);;
  }

  bool move(Layermap& map, Vertexpool<Vertex>& vertexpool){

    ipos = round(pos);                //Position
    n = map.normal(ipos);             //Surface Normal Vector
    surface = map.surface(ipos);      //Surface Composition
    param = soils[surface];           //Surface Composition
    updatefrequency(map, ipos);

    //Modify Parameters Based on Frequency
    param.friction = param.friction*(1.0f-frequency[ipos.y*map.dim.x+ipos.x]);
    evaprate = 0.005f*(1.0f-0.2f*frequency[ipos.y*map.dim.x+ipos.x]);

  //  if(surface == soilmap["Water"] || surface == soilmap["Air"])
  //    return false;

    if(length(vec2(n.x, n.z)*param.friction) < 1E-6)   //No Motion
      return false;

    //Motion Low
    speed = mix(vec2(n.x, n.z), speed, param.friction);
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
    float c_eq = param.solubility*(map.height(ipos)-map.height(pos))*(float)SCALE/80.0f;
    if(c_eq < 0.0) c_eq = 0.0;

    //Execute Transport to Particle
    float cdiff = c_eq - sediment;
    sediment += param.equrate*cdiff*volume;

    //Erode Sediment IN Particle
    if((float)(soils[contains].erosionrate) < frequency[ipos.y*map.dim.x+ipos.x])
      contains = soils[contains].erodes;

    //Add Sediment to Map
    if(cdiff < 0)
      map.add(ipos, map.pool.get(-param.equrate*cdiff*volume, contains));

    //Remove Sediment from Map
    if(cdiff > 0){
      double diff = map.remove(ipos, param.equrate*cdiff*volume);
      while(diff > 0.0) diff = map.remove(ipos, diff);
    }

    //Particle Cascade: Thermal Erosion!
    Particle::cascade(pos, map, vertexpool, 0);

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

    //Add the Soil First!
    map.add(ipos, map.pool.get(param.equrate*sediment*volume, contains));

    sec* SAT =  map.pool.get(volume*volumeFactor, soilmap["Air"]);
    SAT->saturation = 1.0f;
    map.add(ipos, SAT);
    map.update(ipos, vertexpool);
    WaterParticle::cascade(ipos, map, vertexpool, 1);

    return false;

  }

  //This is applied to multiple types of erosion, so I put it in here!
  static void cascade(vec2 pos, Layermap& map, Vertexpool<Vertex>& vertexpool, int transferloop = 0){

    ivec2 ipos = round(pos);

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
      float diff = (map.height(ipos) - map.height(npos))*(float)SCALE/80.0f;
      if(diff == 0)   //No Height Difference
        continue;

      //The Maximum Difference Allowed between two Neighbors
      sec* top = (diff > 0)?map.top(ipos):map.top(npos);
      SurfType type = (diff > 0)?map.surface(ipos):map.surface(npos);
      SurfParam param = soils[type];

      if(top == NULL)
        continue;

      //The Amount of Excess Difference!
      float excess = abs(diff);

      //Maximum Transferrable Amount of Water
      float transfer = excess / 2.0f;

      //Actual Amount of Water Available

      float wheight = (top != NULL)?top->size * param.porosity * top->saturation:0.0f;
      transfer = (wheight < transfer)?wheight:transfer;

      if(transfer <= 0)
        continue;

      bool recascade = false;

      //Cap by Maximum Transferrable Amount
      if(diff > 0){
        double diff = map.remove(ipos, transfer);
        if(diff != 0) recascade = true;
        if(transfer > 0) recascade = true;
        map.add(npos, map.pool.get(transfer, soilmap["Air"]));
        map.top(npos)->saturation = 1.0f;
        map.update(npos, vertexpool);
        map.update(ipos, vertexpool);
      }

      else{
        double diff = map.remove(npos, transfer);
        if(diff != 0) recascade = true;
        if(transfer > 0) recascade = true;
        map.add(ipos, map.pool.get(transfer, soilmap["Air"]));
        map.top(ipos)->saturation = 1.0f;
        map.update(npos, vertexpool);
        map.update(ipos, vertexpool);
      }

      if(recascade && transferloop > 0)
        WaterParticle::cascade(npos, map, vertexpool, --transferloop);

    }

  }

  static void seep(Layermap& map, Vertexpool<Vertex>& vertexpool){

    for(size_t x = 0; x < map.dim.x; x++)
    for(size_t y = 0; y < map.dim.y; y++){

      sec* top = map.top(ivec2(x, y));
      double pressure = 0.0f;            //Pressure Increases Moving Down
      if(top == NULL) continue;

      WaterParticle::cascade(ivec2(x,y), map, vertexpool, 1);
      map.update(ivec2(x,y), vertexpool);

    }

    for(size_t x = 0; x < map.dim.x; x++)
    for(size_t y = 0; y < map.dim.y; y++){

      sec* top = map.top(ivec2(x, y));
      double pressure = 0.0f;            //Pressure Increases Moving Down
      if(top == NULL) continue;

      while(top != NULL && top->prev != NULL){

        sec* prev = top->prev;

        SurfParam param = soils[top->type];
        SurfParam nparam = soils[prev->type];

        // Volume Top Layer
        double vol = top->size*top->saturation*param.porosity;

        //Volume Bottom Layer
        double nvol = prev->size*prev->saturation*nparam.porosity;

        //Empty Volume Bottom Layer
        double nevol = prev->size*(1.0f - prev->saturation)*nparam.porosity;

        double seepage = 0.5f;

        // Compute Pressure
        //pressure *= (1.0f - param.porosity);  //Pressure Drop
        //pressure += vol;                      //Increase
        //Seepage Rate (Top-To-Bottom)
      //	double seepage = 1.0f / (1.0f + pressure * );

        //Transferred Volume is the Smaller Amount!
        double transfer = (vol < nevol) ? vol : nevol;
        if(transfer < 1E-6) seepage = 1.0f; //Just Remove the Rest

        if(transfer >= 0){

          // Remove from Top Layer
          if(top->type == soilmap["Air"])
            double diff = map.remove(ivec2(x,y), seepage*transfer);
          else
            top->saturation -= (seepage*transfer) / (top->size*param.porosity);

          prev->saturation += (seepage*transfer) / (prev->size*nparam.porosity);

        }

        top = prev;

      }

      map.update(ivec2(x,y), vertexpool);

    }

  }

};

float* WaterParticle::frequency = new float[SIZEX*SIZEY]{0.0f};
float* WaterParticle::track = new float[SIZEX*SIZEY]{0.0f};
