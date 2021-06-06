#include <TinyEngine/TinyEngine>
#include <TinyEngine/camera>

#define WIDTH 1200
#define HEIGHT 1000

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
	Tiny::window("Soil Machine", WIDTH, HEIGHT);
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

	//Define Layermap, Construct Vertexpool
	Vertexpool<Vertex> vertexpool(SIZEX*SIZEY, 1);
	Layermap map(SEED, glm::ivec2(SIZEX, SIZEY), vertexpool);

	Tiny::view.interface = [&](){

//		ImGui::ShowDemoWindow();

		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
	  ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);

		ImGui::Begin("Soil Machine Controller");

		if(ImGui::BeginTabBar("SoilMachineTabBar")){

			if(ImGui::BeginTabItem("Map")){

				ImGui::Text("World Seed: "); ImGui::SameLine();
				ImGui::Text("%i", SEED);

				ImGui::EndTabItem();
			}

			if(ImGui::BeginTabItem("Erosion")){



				ImGui::EndTabItem();
			}

			if(ImGui::BeginTabItem("Soil")){

				static SurfType selected = AIR; // Here we store our selection data as an index.
		    const char* label = pdict[selected].name.c_str();  // Label to preview before opening the combo (technically it could be anything)

				if (ImGui::BeginCombo("Select Soil", label)){
					for (std::map<SurfType, SurfParam>::iterator it = pdict.begin(); it != pdict.end(); it++){
						const bool isselected = (selected == it->first);
						if (ImGui::Selectable(it->second.name.c_str(), isselected))
		 					 selected = it->first;
						if(isselected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
			 	}

				//Visualize the Data from the Selected Soil
				if(ImGui::ColorEdit3("Color", &pdict[selected].color[0]))
					map.update(vertexpool);

				ImGui::DragFloat("Density", &pdict[selected].density, 0.0001f, 0.0f, 1.0f);

				if(ImGui::TreeNode("Hydraulic Erosion")){
					ImGui::DragFloat("Water Solubility", &pdict[selected].solubility, 0.0001f, 0.0f, 1.0f);
					ImGui::DragFloat("Equilibriation Rate", &pdict[selected].equrate, 0.0001f, 0.0f, 1.0f);
					ImGui::DragFloat("Surface Friction", &pdict[selected].friction, 0.0001f, 0.0f, 1.0f);
					ImGui::DragFloat("Erosion Rate", &pdict[selected].erosionrate, 0.0001f, 0.0f, 1.0f);
					ImGui::TreePop();
				}

				if(ImGui::TreeNode("Wind Erosion")){
					ImGui::DragFloat("Suspension Rate", &pdict[selected].suspension, 0.0001f, 0.0f, 1.0f);
					ImGui::TreePop();
				}
				if(ImGui::TreeNode("Sediment Cascading")){
					ImGui::DragFloat("Max. Pile Height", &pdict[selected].maxdiff, 0.0001f, 0.0f, 1.0f);
					ImGui::DragFloat("Settling Rate", &pdict[selected].settling, 0.0001f, 0.0f, 1.0f);
					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}

			if(ImGui::BeginTabItem("Visualization")){

				ImGui::Checkbox("Distance Fog", &scene::distancefog);
				ImGui::ColorEdit3("Sky Color", &scene::skycolor[0]);

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();

/*

		  ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

		    //Map Menu

		      //Reset the Heightmap!

		      if(ImGui::Button("Random Initialize")){
		        world.generate();
		      }

		      //World-Map Scale
		      static int scale = world.scale;
		      ImGui::PushItemWidth(200);
		      ImGui::DragInt("Height Scale", &scale, 1);
		      if(ImGui::Button("Remesh")){
		        world.scale = scale;
		        world.updated = true;
		      }

		    }

		    //Erosion Menu
		    if(ImGui::BeginTabItem("Erosion")){


		      //Erosion Step
		      ImGui::PushItemWidth(200);
		      ImGui::DragInt("Erosion Cycles", &world.remaining, 100);
		      ImGui::DragInt("Stepsize", &world.erosionstep, 100);

		      //Start / Stop Button
		      if(ImGui::Button("Run Erosion")){
		        world.active = true;
		      } ImGui::SameLine();
		      if(ImGui::Button("Stop Erosion")){
		        world.active = false;
		      }

		      ImGui::Text("Parameters:");
		      ImGui::DragFloat("Timestep", &world.dt, 0.01f, 0.01f, 5.0f);
		      ImGui::DragFloat("Minimum Volume", &world.minVol, 0.0001f, 0.0001f, 0.1f);
		      ImGui::DragFloat("Density", &world.density, 0.1f, 0.1f, 5.0f);
		      ImGui::DragFloat("Deposition Rate", &world.depositionRate, 0.01f, 0.01f, 1.0f);
		      ImGui::DragFloat("Evaporation Rate", &world.evapRate, 0.001f, 0.001f, 0.5f);
		      ImGui::DragFloat("Friction", &world.friction, 0.001f, 0.001f, 0.5f);

		      ImGui::EndTabItem();
		    }
		    if(ImGui::BeginTabItem("Renderer")){
		      ImGui::DragFloat("Steepness", &view.steepness, 0.05f, 0.0f, 1.0f);

		      static float flat[3] = {view.flatColor.x, view.flatColor.y, view.flatColor.z};
		      ImGui::ColorEdit3("Flat Color", flat);
		      view.flatColor = glm::vec3(flat[0], flat[1], flat[2]);

		      static float steep[3] = {view.steepColor.x, view.steepColor.y, view.steepColor.z};
		      ImGui::ColorEdit3("Steep Color", steep);
		      view.steepColor = glm::vec3(steep[0], steep[1], steep[2]);

		      ImGui::DragFloat("Light Strength", &view.lightStrength, 0.01f, 0.0f, 2.0f);

		      ImGui::DragFloat("Rate", &view.rate, 0.1f, 0.0f, 1.5f);

		      ImGui::EndTabItem();
		    }
		  }
		  ImGui::End();

*/


	};

  //Visualization Shader
  Shader shader({"source/shader/default.vs", "source/shader/default.fs"}, {"in_Position", "in_Normal", "in_Color"});
	Shader depth({"source/shader/depth.vs", "source/shader/depth.fs"}, {"in_Position"});
	Shader effect({"source/shader/effect.vs", "source/shader/effect.fs"}, {"in_Quad", "in_Tex"});

	Billboard image(WIDTH, HEIGHT); 			//1200x1000
	Billboard shadow(2000, 2000, false); 	//800x800, depth only
	Square2D flat;												//For Billboard Rendering

	//Define the rendering pipeline
	Tiny::view.pipeline = [&](){

		//Render Shadowmap
    shadow.target();                  //Prepare Target
    depth.use();                      //Prepare Shader
    depth.uniform("dmvp", scene::dp * scene::dv);
		vertexpool.render(GL_TRIANGLES);

		//Render Image
		image.target(scene::skycolor);
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

		//Render Image with Effects
		Tiny::view.target(scene::skycolor);	//Clear Screen to white
		effect.use();
		effect.uniform("skycolor", scene::skycolor);
		effect.uniform("distancefog", scene::distancefog);
		effect.texture("imageTexture", image.texture);
		effect.texture("depthTexture", image.depth);
		flat.render();

	};

	//Execute the render loop
	Tiny::loop([&](){

		if(paused) return;

		for(int i = 0; i < 1000; i++){
		  WaterParticle particle(vec2(rand()%map.dim.x, rand()%map.dim.y), map);
	    while(particle.move(map, vertexpool) && particle.interact(map, vertexpool));
		}

		for(int i = 0; i < 1000; i++){
			WindParticle particle(vec2(rand()%map.dim.x, rand()%map.dim.y), map);
			while(particle.move(map, vertexpool) && particle.interact(map, vertexpool));
		}

	});

	Tiny::quit();

	return 0;

}
