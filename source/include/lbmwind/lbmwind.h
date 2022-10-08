/*
================================================================================
      LBMWind File: GPU Accelerated Lattice Boltzmann for Wind on Terrain
================================================================================
*/

#ifndef LBMWIND
#define LBMWIND

#include <random>

namespace lbmw {
using namespace glm;
using namespace std;

// Main Simulation Dimension

const int NX = 64;
const int NY = 40;
const int NZ = 64;
const int Q = 19;
vec4 scale = vec4(1);

// Retrievable Storage Buffers

vec4* dirs;
float* boundary;
vector<vec4> pos;

// GPU Buffers

Buffer* dirbuf;
Buffer* f;
Buffer* fprop;
Buffer* rho;
Buffer* v;
Buffer* b;

// Shaders

Compute* init;
Compute* collide;
Compute* stream;

// Flow Visualization

const int NPARTICLE = (2 << 13);
Shader* streamshader;
Compute* move;
Model* windmodel;
Buffer* posbuf;

// Interface Parameters
bool updatewind = false;
bool renderwind = false;

// Functions

void initialize();
void quit();

void update();
void render(mat4);


//

uniform_real_distribution<double> u(0.0, 1.0);
unsigned seed;
mt19937 generator;




void initialize(){

  dirbuf = new Buffer();
  f = new Buffer();
  fprop = new Buffer();
  rho = new Buffer();
  v = new Buffer();
  b = new Buffer();
  posbuf = new Buffer();

  dirs = new vec4[NX*NY*NZ]{vec4(0)};
  dirbuf->fill(NX*NY*NZ, dirs);

  boundary = new float[NX*NY*NZ]{0.0f};
  b->fill(NX*NY*NZ, boundary);

  f->fill(NX*NY*NZ*Q, (float*)NULL);
  fprop->fill(NX*NY*NZ*Q, (float*)NULL);

  rho->fill(NX*NY*NZ, (float*)NULL);       //Density (For Efficiency)

  // Initialize Shaders

  init = new Compute("source/include/lbmwind/shader/LBM/init.cs", {"f", "fprop", "b", "rho", "v"});
  init->bind<float>("f", f);
  init->bind<float>("fprop", fprop);
  init->bind<float>("b", b);
  init->bind<float>("rho", rho);
  init->bind<glm::vec4>("v", dirbuf);

  init->use();
  init->uniform("NX", NX);
  init->uniform("NY", NY);
  init->uniform("NZ", NZ);
  init->dispatch(NX/32, NY, NZ/32);

  collide = new Compute("source/include/lbmwind/shader/LBM/collide.cs", {"f", "fprop", "b", "rho", "v"});
  collide->bind<float>("f", f);
  collide->bind<float>("fprop", fprop);
  collide->bind<float>("b", b);
  collide->bind<float>("rho", rho);
  collide->bind<glm::vec4>("v", dirbuf);

  stream = new Compute("source/include/lbmwind/shader/LBM/stream.cs", {"f", "fprop", "b"});
  stream->bind<float>("f", f);
  stream->bind<float>("fprop", fprop);
  stream->bind<float>("b", b);

  // Setup Particle System

  streamshader = new Shader({"source/include/lbmwind/shader/stream.vs", "source/include/lbmwind/shader/stream.gs", "source/include/lbmwind/shader/stream.fs"}, {"in_Position", "in_Direction"});

  scale = vec4(SIZEX, SCALE, SIZEY, 1)/vec4(NX, 32, NZ, 1);

  seed = chrono::system_clock::now().time_since_epoch().count();
  generator.seed(seed);

  for(size_t i = 0; i < NPARTICLE; i++)
    pos.push_back(glm::vec4(u(generator), u(generator), u(generator), 1)*glm::vec4(NX, NY, NZ, 1));
  posbuf->fill(pos);

  // Model for Rendering Position, Direction Data

  windmodel = new Model({"in_Position", "in_Direction"});
  windmodel->bind<glm::vec4>("in_Position", posbuf);
  windmodel->bind<glm::vec4>("in_Direction", dirbuf);
  windmodel->SIZE = pos.size();

  // Shader to Move Particles Along

  move = new Compute("source/include/lbmwind/shader/move.cs", {"b", "v", "p"});
  move->bind<glm::vec4>("b", b);
  move->bind<glm::vec4>("v", dirbuf);
  move->bind<glm::vec4>("p", posbuf);

}

void quit(){

  delete init;
  delete collide;
  delete stream;

  delete move;
  delete streamshader;

  delete dirs;
  delete boundary;

  delete dirbuf;
  delete f;
  delete fprop;
  delete rho;
  delete v;
  delete b;
  delete posbuf;

}

float t = 0.0f;

void update(){

	t += 0.001f;

  collide->use();
  collide->uniform("NX", NX);
  collide->uniform("NY", NY);
  collide->uniform("NZ", NZ);
  collide->dispatch(NX/32, NY, NZ/32);

  stream->use();
  stream->uniform("NX", NX);
  stream->uniform("NY", NY);
  stream->uniform("NZ", NZ);
  stream->uniform("t", t);
  stream->dispatch(NX/32, NY, NZ/32);

  move->use();
  move->uniform("NX", NX);
  move->uniform("NY", NY);
  move->uniform("NZ", NZ);
  move->dispatch(NPARTICLE/1024);

  posbuf->retrieve(pos);

  for(auto& p: pos){

    if(p.x >= NX-1.0
    || p.y >= NY-1.0
    || p.z >= NZ-1.0
    || p.x <= 0.0
    || p.y <= 0.0
    || p.z <= 0.0
    || boundary[((int)p.x*NY + (int)p.y)*NZ + (int)p.z] > 0.0
    || u(generator) > 0.995
    ){

      p = glm::vec4(u(generator), u(generator), u(generator), 1)*glm::vec4(NX, NY, NZ, 1);
      while( boundary[((int)p.x*NY + (int)p.y)*NZ + (int)p.z] > 0.0)
        p = glm::vec4(u(generator), u(generator), u(generator), 1)*glm::vec4(NX, NY, NZ, 1);
    }

  }

  posbuf->fill(pos);



}

void render( mat4 vp ){

  streamshader->use();
  streamshader->uniform("model", glm::scale(glm::mat4(1.0f), vec3(scale)));			//Set Model Matrix
  streamshader->uniform("vp", vp);
  streamshader->uniform("NX", NX);
  streamshader->uniform("NY", NY);
  streamshader->uniform("NZ", NZ);
  windmodel->render(GL_POINTS);

}

};

#endif
