#include <TinyEngine/TinyEngine>
#include <TinyEngine/camera>
#include <TinyEngine/parse>
#include <TinyEngine/image>

int SIZEX = 256;
int SIZEY = 256;
int SCALE = 80;
int SLICE = 2*SCALE;
int NWIND = 250;
int NWATER = 250;

#define POOLSIZE 10000000
int SEED;

#include "source/vertexpool.hpp"
#include "source/world.hpp"
#include "source/model.hpp"

/*
#include <soillib/external/FastNoiseLite.h>
#include "source/surface.hpp"
#include "source/layermap.hpp"

#include "source/particle/water.hpp"
#include "source/particle/wind.hpp"
*/

#include "source/io.hpp"

int main( int argc, char* args[] ) {

	assert(TINYENGINE_VERSION == "1.7");

	cout<<"Launching SoilMachine V1.1"<<endl;

	Tiny::view.vsync = false;
	Tiny::view.antialias = 0;
	Tiny::window("Soil Machine", WIDTH, HEIGHT);
	glDisable(GL_CULL_FACE);

	// Initialize World

	parse::get(argc, args);

	SEED = time(NULL);
	if(parse::option.contains("SEED"))
		SEED = stoi(parse::option["SEED"]);

	if(parse::option.contains("soil"))
		loadsoil(parse::option["soil"]);
	else loadsoil();

	World world(SEED);

//	WaterParticle::init();
//	WindParticle::init();

	//Initialize a Window
  cam::near = -800.0f;
	cam::far = 800.0f;
	cam::moverate = 10.0f;
  cam::look = glm::vec3(SIZEX/2, SCALE/2, SIZEY/2);
	cam::init(3, cam::ORTHO);

	cam::rot = -45.0f;
	cam::roty = 45.0f;
	cam::update();

	bool dowindcycles = true;
	bool dowatercycles = true;
	bool paused = true;


	Tiny::event.handler = [&](){

		cam::handler();

		if(!Tiny::event.press.empty() && Tiny::event.press.back() == SDLK_p)
			paused = !paused;

	};

	//Define Layermap, Construct Vertexpool
	Vertexpool<Vertex> vertexpool(SIZEX*SIZEY, 1);

	section = vertexpool.section(SIZEX*SIZEY, 0, glm::vec3(0));

	indexnode(world, vertexpool);
	updatenode(world, vertexpool);

	//Layermap map(SEED, glm::ivec2(SIZEX, SIZEY), vertexpool);

	/*
	//Particle Visualization Textures
	Texture watertexture(image::make([&](ivec2 i){
		float wf = WaterParticle::frequency[i.y*SIZEX+i.x];
		return vec4(wf, wf, wf, 1);
	}, ivec2(SIZEX, SIZEY)));

	Texture windtexture(image::make([&](ivec2 i){
		float wf = WindParticle::frequency[i.y*SIZEX+i.x];
		return vec4(wf, wf, wf, 1);
	}, ivec2(SIZEX, SIZEY)));
	*/

  //Visualization Shader
  Shader shader({"source/shader/default.vs", "source/shader/default.fs"}, {"in_Position", "in_Normal", "in_Color", "in_Index"}, {"k"});
	Shader depth({"source/shader/depth.vs", "source/shader/depth.fs"}, {"in_Position"});
	Shader effect({"source/shader/effect.vs", "source/shader/effect.fs"}, {"in_Quad", "in_Tex"});

	// Lighting Parameters for Soil Types
	Buffer phongbuf(phong);
	shader.bind<vec4>("k", &phongbuf);

	Billboard image(WIDTH, HEIGHT); 			//1200x1000


	Texture shadowmap(4000, 4000, {GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT});
  Target shadow(4000, 4000);
  shadow.bind(shadowmap, GL_DEPTH_ATTACHMENT);

	Square2D flat;												//For Billboard Rendering

	//Define the rendering pipeline
	Tiny::view.pipeline = [&](){

		//Render Shadowmap
    shadow.target();                  //Prepare Target
    depth.use();                      //Prepare Shader
    depth.uniform("dmvp", dp * dv);
		vertexpool.render(GL_TRIANGLES);

		//Render Image
		image.target(skycolor);
	  shader.use();
    shader.uniform("model", glm::mat4(1));
		shader.uniform("vp", cam::vp);
		shader.uniform("dbvp", bias*dp*dv);
    shader.uniform("lightcolor", lightcolor);
    shader.uniform("lightstrength", lightstrength);
    shader.uniform("lightpos", lightpos);
    shader.uniform("lookdir", -cam::pos);
		shader.texture("shadowmap", shadowmap);
		shader.uniform("wateroverlay", wateroverlay);
		shader.uniform("watercolor", watercolor);
	//	shader.texture("watermap", watertexture);
		vertexpool.render(GL_TRIANGLES);

		//Render Image with Effects
		Tiny::view.target(skycolor);	//Clear Screen to white
		effect.use();
		effect.uniform("skycolor", skycolor);
		effect.uniform("distancefog", distancefog);
		effect.texture("imageTexture", image.texture);
		effect.texture("depthTexture", image.depth);
		flat.render();


	};

	//Execute the render loop
	Tiny::loop([&](){

		if(paused) return;

		/*

		if(dowatercycles)
		for(int i = 0; i < NWATER; i++){

			WaterParticle particle(map);

			while(true){
				while(particle.move(map, vertexpool) && particle.interact(map, vertexpool));
				if(!particle.flood(map, vertexpool))
					break;
			}

		}

		if(dowatercycles)
		WaterParticle::seep(map, vertexpool);

		if(dowindcycles)
		for(int i = 0; i < NWIND; i++){
			WindParticle particle(map);
			while(particle.move(map, vertexpool) && particle.interact(map, vertexpool));
		}

		//Update Raw Textures
		if(dowatercycles){
			WaterParticle::mapfrequency(map);
			watertexture.raw(image::make([&](ivec2 i){
				float wf = WaterParticle::frequency[i.y*SIZEX+i.x];
				return vec4(wf, wf, wf, 1);
			}, ivec2(SIZEX, SIZEY)));
			WaterParticle::resetfrequency(map);
		}

		if(dowindcycles){
			windtexture.raw(image::make([&](ivec2 i){
				float wf = WindParticle::frequency[i.y*SIZEX+i.x];
				return vec4(wf, wf, wf, 1);
			}, ivec2(SIZEX, SIZEY)));
		}
		*/

	});

	Tiny::quit();

	return 0;

}
