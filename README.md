# SoilMachine

Advanced, modular, coupled geomorpohology simulator for real-time procedural terrain generation.

Article: **Coming Soon(ish)**

## Description

SoilMachine is a unified geomorphology simulator for procedural terrain. It represents the culmination of a lot of work on particle-based transport and erosion systems (**hydraulic and wind**). Most importantly, a new **layered terrain** data structure allows for modeling **surface and sub-surface phenomena** based on **particle transport** with **multiple soil types**, which exceeds the functionality of voxels.

![Banner](https://github.com/weigert/SoilMachine/blob/master/screenshots/banner.png)

SoilMachine is written from scratch in very little code and visualized with [TinyEngine](https://github.com/weigert/TinyEngine) using a technique called vertex pooling, which obviates the need to remesh the heightmap (ever). It is designed to be modular and expandable.

This repo is currently in development and will probably not be done soon but I am publishing it so updates can be followed live for anybody who is interested.

### Screenshots

Sped-up (5x) erosion of sand on rock. Hydraulic erosion moves sand and gravel, and breaks rock into gravel. Wind erosion moves the sand. Sand and gravel both undergo sediment cascading:

![Sand on Rock 1](https://github.com/weigert/SoilMachine/blob/master/screenshots/SoilMachine.gif)

Different Scene:

![Sand on Rock 2](https://github.com/weigert/SoilMachine/blob/master/screenshots/SoilMachine2.gif)

SoilMachine can handle multiple soil types undergoing the same type of erosion. Here is an animation (5x speed) of two different types of sand being moved by the wind:

![Multisand](https://github.com/weigert/SoilMachine/blob/master/screenshots/SoilMachineMultiSand.gif)

SoilMachine lets you define "sediment chains", i.e. rock erodes to gravel erodes to soil. Here is an animation (5x speed) of such a 3-length erosion chain undergoing hydraulic erosion (none of these soil types are effected by wind):

![Multisand](https://github.com/weigert/SoilMachine/blob/master/screenshots/SoilMachine3.gif)

SoilMachine elegantly combines hydraulic and wind erosion into a single structure, so we can show what happens if one effect dominates over another on the same landscape. 

Here is an animation showing 2 million simulated particles on sand, with a varying fraction of wind particles vs. water particles. The landscape continuously morphs between wind dominant (i.e. dunes) vs. water dominant (i.e. mountain-like):

![Wind vs. Water](https://github.com/weigert/SoilMachine/blob/master/screenshots/Hydraulic2Wind.gif)

### Features

**Implemented**

- Vertexpool based heightmap visualization (OpenGL AZDO) to eliminate remesh cost
- Layered terrain data structure using memory pooled, run-length encoded sediment sections as a linked list on a grid
- Modular soil type description and "erosion chains"
- Modular description of transport types using a particle base class
  - Wind erosion particles based on [Particle-Based Wind Erosion](https://github.com/weigert/SimpleWindErosion)
  - Hydraulic erosion particles based on [Particle-Based Erosion](https://github.com/weigert/SimpleErosion)

**Planned**

- Parallelization of the particle base class terrain interaction
- Sediment compaction / conversion
- Water-logging using sediment porosity
- Implementation of multiple water tables based on [Particle-Based Hydrology](https://github.com/weigert/SimpleHydrology)

- GUI Interface for Initial Terrain
- GUI Interface for Defining Sediment Types
- Data Export

## Utilization

Tested on Ubuntu20 LTS.

### Building & Running

SoilMachine is visualized using `TinyEngine`. Install `TinyEngine` using the instructions at the repo:

    https://github.com/weigert/TinyEngine
    
TinyEngine is the only dependency (besides sub-dependencies).

Once that works, build SoilMachine using the makefile and run in place (no install):

    cd ~
    git clone https://github.com/weigert/SoilMachine
    cd SoilMachine
    make all
    ./soilmachine

### Controls

The GUI is currently being overhauled.

    P: Toggle Pause
  
    Scroll-Y: Zoom
    Scroll-X: Camera Angle
    Up / Down Arrow: Camera Tilt
    C / V: Move Camera Up / Down
    WASD: Move Camera in Plane
    
### License

MIT License
