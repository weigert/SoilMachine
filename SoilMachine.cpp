#include <TinyEngine/TinyEngine>
#include <TinyEngine/camera>

#define SIZEX 512
#define SIZEY 512
#define SCALE 128

#include "source/include/vertexpool.h"
#include "source/include/scene.h"

#include "source/layermap.h"
#include "source/particle/water.h"
#include "source/particle/wind.h"

int main( int argc, char* args[] ) {

	cout<<"Launching SoilMachine V1.0"<<endl;

	//Get Seed
	srand(time(NULL));

	int SEED;
	if(argc > 1)
		SEED = stoi(args[1]);
	else
		SEED = rand();

	cout<<"SEED: "<<SEED<<endl;

	//Initialize a Window
	Tiny::window("Soil Machine", 1200, 1000);
  cam::near = -800.0f;
	cam::far = 800.0f;
	cam::moverate = 10.0f;
  cam::look = glm::vec3(SIZEX/2, SCALE/2, SIZEY/2);
	cam::init(3, cam::ORTHO);

	cam::rot = 45.0f;
	cam::roty = 45.0f;
	cam::update();


	bool paused = true;

	Tiny::event.handler = [&](){

		cam::handler();

		if(!Tiny::event.press.empty() && Tiny::event.press.back() == SDLK_p)
			paused = !paused;

	};

	Tiny::view.interface = [&](){};

	//Define Layermap, Construct Vertexpool
	Vertexpool<Vertex> vertexpool(SIZEX*SIZEY, 1);
	Layermap map(SEED, glm::ivec2(SIZEX, SIZEY), vertexpool);

  //Visualization Shader
  Shader shader({"source/shader/default.vs", "source/shader/default.fs"}, {"in_Position", "in_Normal", "in_Color"});
	Shader depth({"source/shader/depth.vs", "source/shader/depth.fs"}, {"in_Position"});

	Billboard shadow(2000, 2000, false); //800x800, depth only

	//Define the rendering pipeline
	Tiny::view.pipeline = [&](){

		//Render Shadowmap
    shadow.target();                  //Prepare Target
    depth.use();                      //Prepare Shader
    depth.uniform("dmvp", scene::dp * scene::dv);
		vertexpool.render(GL_TRIANGLES);

		Tiny::view.target(scene::skycolor);	//Clear Screen to white
    shader.use();

    shader.uniform("model", glm::mat4(1));
		shader.uniform("vp", cam::vp);
		shader.uniform("dbvp", scene::bias*scene::dp*scene::dv);

    shader.uniform("lightcolor", scene::lightcolor);
    shader.uniform("lightstrength", scene::lightstrength);
    shader.uniform("lightpos", scene::lightpos);
    shader.uniform("lookdir", -cam::pos);
		shader.texture("shadowmap", shadow.depth);

		vertexpool.render(GL_TRIANGLES);

	};

	//Execute the render loop
	Tiny::loop([&](){

		if(paused) return;

		for(int i = 0; i < 1500; i++){
		  WaterParticle particle(vec2(rand()%map.dim.x, rand()%map.dim.y), map);
	    while(particle.move(map, vertexpool) && particle.interact(map, vertexpool));
		}

		for(int i = 0; i < 500; i++){
			WindParticle particle(vec2(rand()%map.dim.x, rand()%map.dim.y), map);
			while(particle.move(map, vertexpool) && particle.interact(map, vertexpool));
		}

	});

	Tiny::quit();

	return 0;

}
