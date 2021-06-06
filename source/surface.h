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

  {AIR, { 0.0f, vec4(0.27, 0.57, 0.6, 0.0),
    AIR, 1.0f, 1.0f, 1.0f,
    AIR, 0.0f,
    AIR, 1.0f, 0.95f,
    AIR, 0.0f, 0.0f}},

//  {WATER, {0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.1f, WATER, WATER, vec4(0.27, 0.57, 0.6, 1.0)}},

  {REDSAND, { 0.4f, vec4(0.86, 0.52, 0.34, 1.0), //vec4(0.88, 0.79, 0.41, 1.0)
    REDSAND, 0.2f, 0.5f, 0.5f,
    REDSAND, 0.0f,
    REDSAND, 0.0005f, 0.01f,
    REDSAND, 0.002f, 0.0f}},

  {SAND, { 0.4f, vec4(0.88, 0.79, 0.41, 1.0), //
    SAND, 1.0f, 0.8f, 0.2f,
    SAND, 0.0f,
    SAND, 0.001f, 0.01f,
    SAND, 0.001f, 0.0f}},

  {SANDSTONE, { 1.0f, vec4(0.66, 0.38, 0.22, 1.0),
    REDSAND, 0.8f, 0.8f, 0.5f,
    REDSAND, 0.0f,
    REDSAND, 10.0f, 0.0f,
    REDSAND, 0.0f, 0.0f}},

  //No Rock-Cascading, Only Erosion

  {ROCK, { 1.0f, vec4(0.5, 0.5, 0.5, 1.0),
    GRAVEL, 0.5f, 0.4f, 0.95f,        //Hydrological Transport
    GRAVEL, 0.0f,                     //Hydraulic In-Particle Erosion
    GRAVEL, 10.0f, 0.0f,              //Cascading Height, Rate, Species
    GRAVEL, 0.0f, 0.0f}},             //Wind-Erosion Suspendibility, Abrasion, Type

  {GRAVEL, { 0.95f, vec4(0.7, 0.7, 0.7, 1.0),
    GRAVEL, 0.75f, 0.6f, 0.75f,
    SOIL, 0.01f,
    GRAVEL, 0.0005f, 0.8f,
    GRAVEL, 0.0f, 0.0f, }},

  {SOIL, { 0.7f, vec4(0.32, 0.52, 0.32, 1.0),
    SOIL, 0.75f, 0.5f, 0.5f,
    SOIL, 0.0f,
    SOIL, 0.0005f, 0.8f,
    SOIL, 0.0f, 0.0f}},


};
