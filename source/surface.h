/*
================================================================================
              Description of Surface Types and their Properties
================================================================================
*/

// Surface Type Enumerator

#include <map>

enum SurfType {
  AIR, ROCK, SOIL, SAND, WATER
};

struct SurfParam {

  //Transport Parameters
  float friction = 1.0f;     //Surface Friction, Water Particles
  float solubility = 1.0f;   //Relative Solubility in Water
  float equrate = 1.0f;      //Rate of Reaching Equilibrium
  float maxdiff = 1.0f;      //Maximum Settling Height Difference
  float settling = 0.2f;     //Settling Rate

  //Erosion Chain
  SurfType becomes;

  //Visualization Parameters
  vec4 color = vec4(0.5, 0.5, 0.5, 1.0);

};

std::map<SurfType, SurfParam> pdict = {

  {AIR, {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, AIR, vec4(1.0, 0.0, 0.0, 0.0)}},
  {ROCK, {0.95f, 0.1f, 0.2f, 0.001f, 0.8f, SOIL, vec4(0.4, 0.4, 0.4, 1.0)}},
  {SOIL, {0.95f, 1.0f, 0.6f, 0.0001f, 0.2f, SOIL, vec4(0.32, 0.52, 0.32, 1.0)}},
  {SAND, {0.95f, 1.0f, 0.8f, 10.0f, 0.9f, SAND, vec4(0.86, 0.74, 0.39, 1.0)}},
  {WATER, {0.5f, 0.0f, 0.0f, 0.0f, 1.0f, WATER, vec4(0.27, 0.57, 0.6, 1.0)}}

};
