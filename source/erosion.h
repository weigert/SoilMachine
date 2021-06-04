/*

Super Basic Erosion Implementation!

Just a test to see how we can determine different parameters on the different soil layers.

*/

using namespace glm;

struct Particle{

  Particle(vec2 _pos){ pos = _pos; }

  vec2 pos;
  vec2 speed = vec2(0);

  float volume = 1.0;   //This will vary in time
  float sediment = 0.0; //Fraction of Volume that is Sediment!

};


const float dt = 1.0f;
const double settling = 0.2
;
const float density = 1.0;  //This gives varying amounts of inertia and stuff...
const float evapRate = 0.01;
const float depositionRate = 0.5;
const float minVol = 0.005;
const float friction = 0.8f;

float getmaxdiff(SurfType type){
  if(type == SOIL) return 0.0005f;
  else return 0.005f;  //Large Angle
}

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
    float roughness = getmaxdiff(type);

    //The Amount of Excess Difference!
    float excess = abs(diff) - roughness;
    if(excess <= 0) continue; //No Excess

    //Actual Amount Transferred
    float transfer = excess / 2.0f;

    //Cap by Maximum Transferrable Amount
    if(diff > 0){
      transfer = (transfer < map.top(ipos)->size)?transfer:map.top(ipos)->size;
      map.remove(ipos, dt*settling*transfer);
      map.add(npos, map.pool.get(dt*settling*transfer, SOIL));
    }
    else{
      transfer = (transfer < map.top(npos)->size)?transfer:map.top(npos)->size;
      map.remove(npos, dt*settling*transfer);
      map.add(ipos, map.pool.get(dt*settling*transfer, SOIL));
    }

  }

}

void erode(Layermap& map, Vertexpool<Vertex>& vertexpool, const int ncycles){

  for(int i = 0; i < ncycles; i++){

  vec2 newpos = glm::vec2(rand()%map.dim.x, rand()%map.dim.y);

  Particle drop(newpos);
  vec2 oldpos;
  ivec2 ipos;

  while(drop.volume > minVol){

    if(!glm::all(glm::greaterThanEqual(drop.pos, vec2(0))) ||
       !glm::all(glm::lessThan(drop.pos, (vec2)map.dim-1.0f))) break;

    ipos = round(drop.pos);
    vec3 n = map.normal(ipos);

    //If the Particle is not accelerating...
    if(length(vec2(n.x,n.z)/drop.volume) < 0.05) break;

    //Variable Time-Step
    drop.speed = mix(drop.speed, vec2(n.x, n.z)/(drop.volume*density), friction);//F = ma, so a = F/m
    drop.speed = sqrt(2.0f)*normalize(drop.speed);
    drop.pos   += drop.speed;

    if(!glm::all(glm::greaterThanEqual(drop.pos, vec2(0))) ||
       !glm::all(glm::lessThan(drop.pos, (vec2)map.dim-1.0f))) break;

    float c_eq = length(vec2(n.x,n.z)/drop.volume)*drop.volume*(map.height(ipos)-map.height(drop.pos));

    if(c_eq < 0.0) c_eq = 0.0;
    float cdiff = c_eq - drop.sediment;

    drop.sediment += dt*depositionRate*cdiff;

    if(cdiff < 0)
      map.add(ipos, map.pool.get(-drop.volume*depositionRate*cdiff, SOIL));
    if(cdiff > 0){
      double diff = map.remove(ipos, drop.volume*depositionRate*cdiff);
      while(diff > 0.0) diff = map.remove(ipos, diff);
    }

    cascade(drop.pos, map, vertexpool);
    map.update(ipos, vertexpool);

    drop.sediment /= (1.0-evapRate);
    drop.volume *= (1.0-evapRate);

  }

  }

}
