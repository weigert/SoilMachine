/*
================================================================================
              Description of Surface Types and their Properties
================================================================================
*/

// Surface Type Enumerator

#include <map>

enum SurfType {
  AIR, ROCK, SOIL, SAND, WATER, GRAVEL, SANDSTONE, REDSAND
};

struct SurfParam {

  //General Parameters
  string name;
  float density;
  vec4 color = vec4(0.5, 0.5, 0.5, 1.0);

  //Hydrologyical Parameters
  SurfType transports;        //Surface Type it Transports As
  float solubility = 1.0f;    //Relative Solubility in Water
  float equrate = 1.0f;       //Rate of Reaching Equilibrium
  float friction = 1.0f;      //Surface Friction, Water Particles

  SurfType erodes;            //Surface Type it Erodes To
  float erosionrate = 0.0f;   //Rate of Conversion

  //Cascading Parameters
  SurfType cascades;          //Surface Type it Cascades To
  float maxdiff = 1.0f;       //Maximum Settling Height Difference
  float settling;             //Settling Rate

  //Wind Erosion
  SurfType abrades;           //Surface Type it Cascades To
  float suspension = 0.0001f;
  float abrasion;             //Rate at which particle size decreases

};

std::map<SurfType, SurfParam> pdict = {

  {AIR, { "Air", 0.0f, vec4(0.27, 0.57, 0.6, 0.0),
    AIR, 1.0f, 1.0f, 1.0f,
    AIR, 0.0f,
    AIR, 1.0f, 0.95f,
    AIR, 0.0f, 0.0f}},

  {WATER, { "Water", 1.0f, vec4(0.38, 0.53, 0.68, 1.0),
    WATER, 0.0f, 0.0f, 0.0f,
    WATER, 0.0f,
    WATER, 0.0f, 1.0f,
    WATER, 0.0f, 0.0f}},

  {REDSAND, { "Red Sand", 0.4f, vec4(0.866, 0.635, 0.431, 1.0), //vec4(0.88, 0.79, 0.41, 1.0)
    REDSAND, 0.2f, 0.5f, 0.5f,
    REDSAND, 0.0f,
    REDSAND, 0.005f, 0.05f,
    REDSAND, 0.01f, 0.0f}},

  {SAND, { "Sand", 0.4f, vec4(0.88, 0.79, 0.41, 1.0), //
    SAND, 1.0f, 0.8f, 0.2f,
    SAND, 0.0f,
    SAND, 0.001f, 0.01f,
    SAND, 0.001f, 0.0f}},

  {SANDSTONE, { "Sandstone", 1.0f, vec4(0.66, 0.38, 0.22, 1.0),
    REDSAND, 0.8f, 0.8f, 0.5f,
    REDSAND, 0.0f,
    REDSAND, 10.0f, 0.0f,
    REDSAND, 0.0f, 0.0f}},

  //No Rock-Cascading, Only Erosion

  {ROCK, { "Rock", 1.0f, vec4(0.2, 0.2, 0.2, 1.0),// vec4(0.278, 0.219, 0.219, 1.0),
    GRAVEL, 0.7f, 0.6f, 0.95f,        //Hydrological Transport
    GRAVEL, 0.0f,                     //Hydraulic In-Particle Erosion
    GRAVEL, 0.005f, 0.2f,              //Cascading Height, Rate, Species
    GRAVEL, 0.0f, 0.0f}},             //Wind-Erosion Suspendibility, Abrasion, Type

  {GRAVEL, { "Gravel", 0.95f, vec4(0.75, 0.75, 0.75, 1.0),//vec4(0.447, 0.384, 0.345, 1.0),
    GRAVEL, 0.8f, 0.7f, 0.75f,
    SOIL, 0.01f,
    GRAVEL, 0.01f, 0.5f,
    GRAVEL, 0.0f, 0.0f, }},

    {SOIL, { "Soil", 0.7f, vec4(0.32, 0.52, 0.32, 1.0),
      SOIL, 1.0f, 0.8f, 0.5f,
      SOIL, 0.0f,
      SOIL, 0.0005f, 0.8f,
      SOIL, 0.0f, 0.0f}},

};
