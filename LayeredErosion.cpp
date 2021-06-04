#include <TinyEngine/TinyEngine>
#include <TinyEngine/camera>

#define SEED 125
#define SIZEX 512
#define SIZEY 512
#define scale 120.0f

#include "source/vertexpool.h"
#include "source/layermap.h"
#include "source/erosion.h"
#include "source/scene.h"

int main( int argc, char* args[] ) {

	//Initialize a Window
	Tiny::window("Layered Erosion", 1200, 800);
  cam::near = -800.0f;
	cam::far = 800.0f;
  cam::look = glm::vec3(SIZEX/2, scale/2, SIZEY/2);
	cam::init(10, cam::ORTHO);

	bool paused = true;
	Tiny::event.handler = [&](){

		cam::handler();

		if(!Tiny::event.press.empty() && Tiny::event.press.back() == SDLK_p)
			paused = !paused;

	};
	Tiny::view.interface = [&](){};

  //Define Layermap, Construct Vertexpool
  Vertexpool<Vertex> vertexpool(SIZEX*SIZEY, 1);
  Layermap map(glm::ivec2(SIZEX, SIZEY), vertexpool);

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
    erode(map, vertexpool, 1500);

	});

	Tiny::quit();

	return 0;

}
