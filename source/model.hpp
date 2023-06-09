#ifndef SOILMACHINE_MODEL
#define SOILMACHINE_MODEL

const int WIDTH = 1200;
const int HEIGHT = 1000;

glm::vec3 lightcolor = glm::vec3(1);
float lightstrength = 1.0f;
glm::vec3 lightpos = glm::vec3(1.25f,1.5f,2.5f);
glm::vec3 skycolor = glm::vec3(0.80,0.90,0.89);

bool distancefog = true;

glm::mat4 dp = glm::ortho<float>(-1200, 1200, -1200, 1200, -800, 800);
glm::mat4 dv = glm::lookAt(lightpos, cam::look, glm::vec3(0,1,0));
glm::mat4 bias = glm::mat4(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
);

glm::vec3 watercolor = glm::vec3(0.27, 0.5, 0.7);
bool wateroverlay = true;

uint* section = NULL;                     //Vertexpool Section Pointer

// Meshing, Updating

void indexnode(World& world, Vertexpool<Vertex>& vertexpool){

  for(const auto& [_cell, pos]: world.map){
    if(world.map.oob(pos + ivec2(1,0))) continue;
    if(world.map.oob(pos + ivec2(0,1))) continue;
    if(world.map.oob(pos + ivec2(1,1))) continue;
    vertexpool.indices.push_back(soil::index::flat::flatten(pos + ivec2(0, 0), world.map.dimension));
    vertexpool.indices.push_back(soil::index::flat::flatten(pos + ivec2(0, 1), world.map.dimension));
    vertexpool.indices.push_back(soil::index::flat::flatten(pos + ivec2(1, 0), world.map.dimension));
    vertexpool.indices.push_back(soil::index::flat::flatten(pos + ivec2(1, 0), world.map.dimension));
    vertexpool.indices.push_back(soil::index::flat::flatten(pos + ivec2(0, 1), world.map.dimension));
    vertexpool.indices.push_back(soil::index::flat::flatten(pos + ivec2(1, 1), world.map.dimension));
  }

  vertexpool.resize(section, vertexpool.indices.size());
  vertexpool.index();
  vertexpool.update();

}

void updatenode(World& world, Vertexpool<Vertex>& vertexpool){

  for(auto [cell, pos]: world.map){

    sec* top = cell.top;

    while(top != NULL && top->floor > (float)SLICE/(float)SCALE)
      top = top->prev;

    if(top == NULL){
      vertexpool.fill(section, soil::index::flat::flatten(pos, world.map.dimension),
        vec3(pos.x, 0, pos.y),
        vec3(0,1,0),
        soils[soilmap["Air"]].color,
        soilmap["Air"]
      );
    }

    else if(top->floor + top->size > (float)SLICE/(float)SCALE){

      if(top->floor + top->size*top->saturation > (float)SLICE/(float)SCALE)
      vertexpool.fill(section, soil::index::flat::flatten(pos, world.map.dimension),
        vec3(pos.x, SLICE, pos.y),
        vec3(0,1,0),
    //    normal(p),
        mix(soils[soilmap["Air"]].color, soils[top->type].color, 0.6),
        soilmap["Air"]
      );

      else
      vertexpool.fill(section, soil::index::flat::flatten(pos, world.map.dimension),
        vec3(pos.x, SLICE, pos.y),
        vec3(0,1,0),
  //    normal(p),
        soils[top->type].color,
        top->type
      );

    }

    else{


      vertexpool.fill(section, soil::index::flat::flatten(pos, world.map.dimension),
        vec3(pos.x, SCALE*(top->floor + top->size), pos.y),
        world.normal(pos),
        soils[top->type].color,
        top->type
      );

    }

  }

}

/*
void slice(Vertexpool<Vertex>& vertexpool, double s = SCALE){

  for(int i = 0; i < dim.x; i++)
  for(int j = 0; j < dim.y; j++){

    ivec2 p = ivec2(i,j);

    //Find the first element which starts below the scale!
    sec* top = dat[p.x*dim.y+p.y];
    while(top != NULL && top->floor > s/SCALE)
      top = top->prev;

    if(top == NULL){
      vertexpool.fill(section, p.x*dim.y+p.y,
        vec3(p.x, 0, p.y),
        vec3(0,1,0),
        soils[soilmap["Air"]].color,
        soilmap["Air"]
      );
    }

    else if(top->floor + top->size > s/SCALE){
      if(top->floor + top->size*top->saturation > s/SCALE)
      vertexpool.fill(section, p.x*dim.y+p.y,
        vec3(p.x, s, p.y),
        vec3(0,1,0),
        mix(vec4(1,0,0,1), soils[top->type].color, 0.6),
        top->type
      );
      else
      vertexpool.fill(section, p.x*dim.y+p.y,
        vec3(p.x, s, p.y),
        vec3(0,1,0),
        soils[top->type].color,
        top->type
      );
    }

    else{
      if(top->saturation == 0)  //Fill Watertable!
      vertexpool.fill(section, p.x*dim.y+p.y,
        vec3(p.x, SCALE*(top->floor + top->size), p.y),
        normal(p),
        vec4(1,0,0,1),
        top->type
      );
      else
      vertexpool.fill(section, p.x*dim.y+p.y,
        vec3(p.x, SCALE*(top->floor + top->size), p.y),
        normal(p),
        soils[top->type].color,
        top->type
      );
    }

  }
}
*/

#endif
