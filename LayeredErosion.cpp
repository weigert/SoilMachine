#include <TinyEngine/TinyEngine>
#include <TinyEngine/camera>

#define SEED 110
#define SIZEX 256
#define SIZEY 256
#define scale 80.0f

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

	Tiny::event.handler = cam::handler;
	Tiny::view.interface = [&](){};

  //Define Layermap, Construct Vertexpool
  Vertexpool<Vertex> vertexpool(SIZEX*SIZEY, 1);
  Layermap map(glm::ivec2(SIZEX, SIZEY), vertexpool);

  //Visualization Shader
  Shader shader({"source/shader/default.vs", "source/shader/default.fs"}, {"in_Position", "in_Normal", "in_Color"});
  //Shader depth()

	//Define the rendering pipeline
	Tiny::view.pipeline = [&](){

		Tiny::view.target(glm::vec3(0));	//Clear Screen to white
    shader.use();

    shader.uniform("model", glm::mat4(1));
		shader.uniform("vp", cam::vp);

    shader.uniform("lightcolor", scene::lightcolor);
    shader.uniform("lightstrength", scene::lightstrength);
    shader.uniform("lightpos", scene::lightpos);
    shader.uniform("lookdir", -cam::pos);

		vertexpool.render(GL_TRIANGLES);

	};

	//Execute the render loop
	Tiny::loop([&](){

    erode(map, vertexpool);

	});

	Tiny::quit();

	return 0;

}
