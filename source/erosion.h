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

//Simulation Parameters

/*
float dt = 0.5f;
const float density = 1.0;  //This gives varying amounts of inertia and stuff...
const float evapRate = 0.001;
const float depositionRate = 0.3;
const float minVol = 0.01;
const float friction = 0.05;
*/

float dt = 0.4f;
const float density = 1.0;  //This gives varying amounts of inertia and stuff...
const float evapRate = 0.001;
const float depositionRate = 0.3;
const float minVol = 0.01;
const float friction = 0.05;

void erode(Layermap& map, Vertexpool<Vertex>& vertexpool){

  for(int i = 0; i < 250; i++){

  vec2 newpos = glm::vec2(rand()%map.dim.x, rand()%map.dim.y);

  Particle drop(newpos);
  ivec2 ipos;

  while(drop.volume > minVol){

    ipos = round(drop.pos);
    vec3 n = map.normal(ipos);

    drop.speed += dt*vec2(n.x, n.z)/(drop.volume*density);//F = ma, so a = F/m
    drop.pos   += dt*drop.speed;
    drop.speed *= (1.0-dt*friction);       //Friction Factor

    if(!glm::all(glm::greaterThanEqual(drop.pos, vec2(0))) ||
       !glm::all(glm::lessThan(drop.pos, (vec2)map.dim-1.0f))) break;

    float c_eq = drop.volume*glm::length(drop.speed)*(map.height(ipos)-map.height((ivec2)drop.pos));
    if(c_eq < 0.0) c_eq = 0.0;
    float cdiff = c_eq - drop.sediment;

    drop.sediment += dt*depositionRate*cdiff;

    if(cdiff > 0) map.add(ipos, map.pool.get(-dt*drop.volume*depositionRate*cdiff, SOIL));
    if(cdiff < 0) map.remove(ipos, dt*drop.volume*depositionRate*cdiff);

    if(map.surface(ipos) == AIR){
      map.add(ipos, map.pool.get(0.001f, ROCK));
    }

    drop.sediment /= (1.0-dt*evapRate);
    drop.volume *= (1.0-dt*evapRate);

    map.update(ipos, vertexpool);

  }

  }

}
