#include <TinyEngine/TinyEngine>
#include <TinyEngine/camera>
#include <TinyEngine/parse>
#include <TinyEngine/image>

#define WIDTH 1200
#define HEIGHT 1000

int SIZEX = 256;
int SIZEY = 256;
int SCALE = 80;
int SLICE = 2*SCALE;
int NWIND = 250;
int NWATER = 250;

#define POOLSIZE 10000000
int SEED;

#include "source/include/vertexpool.h"
#include "source/include/scene.h"

#include "source/layermap.h"
#include "source/particle/water.h"
#include "source/particle/wind.h"

#include "source/io.h"

#include "source/include/lbmwind/lbmwind.h"

int main( int argc, char* args[] ) {

	cout<<"Launching SoilMachine V1.1"<<endl;

	parse::get(argc, args);

	srand(time(NULL));
	if(parse::option.contains("SEED"))
		SEED = stoi(parse::option["SEED"]);
	else SEED = rand();
	cout<<"SEED: "<<SEED<<endl;
	srand(SEED);														//Re-Seed

	if(parse::option.contains("soil"))
		loadsoil(parse::option["soil"]);
	else loadsoil();

	WaterParticle::init();
	WindParticle::init();

	//Initialize a Window
	Tiny::view.vsync = false;
	Tiny::view.lineWidth = 2.0f;
	Tiny::view.antialias = 0;

	Tiny::window("Soil Machine", WIDTH, HEIGHT);
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

	glDisable(GL_CULL_FACE);

	Tiny::event.handler = [&](){

		cam::handler();

		if(!Tiny::event.press.empty() && Tiny::event.press.back() == SDLK_p)
			paused = !paused;

	};

	//Define Layermap, Construct Vertexpool
	Vertexpool<Vertex> vertexpool(SIZEX*SIZEY, 1);
	Layermap map(SEED, glm::ivec2(SIZEX, SIZEY), vertexpool);

	//Particle Visualization Textures
	Texture watertexture(image::make([&](int i){
		float wf = WaterParticle::frequency[i];
		return vec4(wf, wf, wf, 1);
	}, vec2(SIZEX, SIZEY)));

	Texture windtexture(image::make([&](int i){
		float wf = WindParticle::frequency[i];
		return vec4(wf, wf, wf, 1);
	}, vec2(SIZEX, SIZEY)));

	Tiny::view.interface = [&](){

//		ImGui::ShowDemoWindow();

		ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_Once);
	  ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Once);

		ImGui::Begin("Soil Machine Controller");

		if(ImGui::BeginTabBar("SoilMachineTabBar")){

			if(ImGui::BeginTabItem("Map")){

				ImGui::Text("World Seed: "); ImGui::SameLine();
				ImGui::DragInt("Seed", &SEED, 1, 0, 100000000);
				if(ImGui::Button("Re-Seed")){
					map.initialize(SEED, ivec2(SIZEX, SIZEY));
					map.meshpool(vertexpool);
				}

				ImGui::Text("Memory Pool Usage: %f%%", 100.0*((double)POOLSIZE-(double)map.pool.free.size())/(double)POOLSIZE);

				ImGui::SliderInt("World Scale", &SCALE, 15, 250);
				if(ImGui::SliderInt("World Slice", &SLICE, 0, 2*SCALE)){
					map.update(vertexpool);
				}

				ImGui::EndTabItem();
			}

			if(ImGui::BeginTabItem("Erosion")){

				if(ImGui::TreeNode("Hydraulic Erosion")){
					ImGui::Checkbox("Do Water Cycles?", &dowatercycles);
					ImGui::DragInt("Particles per Frame", &NWATER, 1, 0, 2000);
					ImGui::Checkbox("Overlay Map?", &scene::wateroverlay);
					ImGui::Text("Frequency Texture: ");
					ImGui::Image((void*)(intptr_t)watertexture.texture, ImVec2(SIZEX, SIZEY));
					ImGui::TreePop();
				}

				if(ImGui::TreeNode("WindErosion")){
					ImGui::Checkbox("Do Wind Cycles?", &dowindcycles);
					ImGui::DragInt("Particles per Frame", &NWIND, 1, 0, 2000);
					ImGui::Text("Frequency Texture: ");
					ImGui::Image((void*)(intptr_t)windtexture.texture, ImVec2(SIZEX, SIZEY));
					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}

			if(ImGui::BeginTabItem("Soil")){

				static SurfType selected = soilmap["Air"]; // Here we store our selection data as an index.
		    const char* label = soils[selected].name.c_str();  // Label to preview before opening the combo (technically it could be anything)

				if (ImGui::BeginCombo("Select Soil", label)){

					for(size_t i = 0; i < soils.size(); i++){
						const bool isselected = (selected == i);
						if (ImGui::Selectable(soils[i].name.c_str(), isselected))
		 					 selected = i;
						if(isselected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
			 	}

				//Visualize the Data from the Selected Soil
				if(ImGui::ColorEdit3("Color", &soils[selected].color[0]))
					map.update(vertexpool);

				ImGui::DragFloat("Density", &soils[selected].density, 0.0001f, 0.0f, 1.0f);

				if(ImGui::TreeNode("Hydraulic Erosion")){
					ImGui::DragFloat("Water Solubility", &soils[selected].solubility, 0.0001f, 0.0f, 1.0f);
					ImGui::DragFloat("Equilibriation Rate", &soils[selected].equrate, 0.0001f, 0.0f, 1.0f);
					ImGui::DragFloat("Surface Friction", &soils[selected].friction, 0.0001f, 0.0f, 1.0f);
					ImGui::DragFloat("Erosion Rate", &soils[selected].erosionrate, 0.0001f, 0.0f, 1.0f);
					ImGui::TreePop();
				}

				if(ImGui::TreeNode("Wind Erosion")){
					ImGui::DragFloat("Suspension Rate", &soils[selected].suspension, 0.0001f, 0.0f, 1.0f);
					ImGui::TreePop();
				}
				if(ImGui::TreeNode("Sediment Cascading")){
					ImGui::DragFloat("Max. Pile Height", &soils[selected].maxdiff, 0.0001f, 0.0f, 1.0f);
					ImGui::DragFloat("Settling Rate", &soils[selected].settling, 0.0001f, 0.0f, 1.0f);
					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}

			if(ImGui::BeginTabItem("Wind")){

				ImGui::Checkbox("Update Wind", &lbmw::updatewind);
				ImGui::Checkbox("Render Wind", &lbmw::renderwind);

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

	};

  //Visualization Shader
  Shader shader({"source/shader/default.vs", "source/shader/default.fs"}, {"in_Position", "in_Normal", "in_Color", "in_Index"}, {"k"});
	Shader depth({"source/shader/depth.vs", "source/shader/depth.fs"}, {"in_Position"});
	Shader effect({"source/shader/effect.vs", "source/shader/effect.fs"}, {"in_Quad", "in_Tex"});

	// Lighting Parameters for Soil Types
	Buffer phongbuf(phong);
	shader.bind<vec4>("k", &phongbuf);

	Billboard image(WIDTH, HEIGHT); 			//1200x1000
	Billboard shadow(4000, 4000, false); 	//800x800, depth only
	Square2D flat;												//For Billboard Rendering

	lbmw::initialize();
	for(size_t x = 0 ; x < lbmw::NX; x++)
	for(size_t y = 0 ; y < lbmw::NY; y++)
	for(size_t z = 0 ; z < lbmw::NZ; z++)
		lbmw::boundary[(x*lbmw::NY + y)*lbmw::NZ + z] = map.height(ivec2(lbmw::scale.x*x, lbmw::scale.z*z)) > (lbmw::scale.y*y)/(float)SCALE;
	lbmw::b->fill(lbmw::NX*lbmw::NY*lbmw::NZ, lbmw::boundary);


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
		shader.uniform("wateroverlay", scene::wateroverlay);
		shader.uniform("watercolor", scene::watercolor);
		shader.texture("watermap", watertexture);
		vertexpool.render(GL_TRIANGLES);

		if(lbmw::renderwind)
			lbmw::render(cam::vp);

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

		if(lbmw::updatewind)
			lbmw::update();

		//Update Raw Textures
		if(dowatercycles){
			WaterParticle::mapfrequency(map);
			watertexture.raw(image::make([&](int i){
				float wf = WaterParticle::frequency[i];
				return vec4(wf, wf, wf, 1);
			}, vec2(SIZEX, SIZEY)));
			WaterParticle::resetfrequency(map);
		}

		if(dowindcycles){
			windtexture.raw(image::make([&](int i){
				float wf = WindParticle::frequency[i];
				return vec4(wf, wf, wf, 1);
			}, vec2(SIZEX, SIZEY)));
		}

	});

	if(parse::option.contains("oc"))
		exportcolor(map, vertexpool, parse::option["oc"]);

	if(parse::option.contains("oh"))
		exportheight(map, vertexpool, parse::option["oh"]);

	lbmw::quit();
	Tiny::quit();

	return 0;

}
