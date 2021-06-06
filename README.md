# SoilMachine

Advanced, modular, coupled geomorpohology simulator for real-time procedural terrain generation.

Article: **Coming Soon(ish)**

## Description

SoilMachine is a unified geomorphology simulator for procedural terrain. It represents the culmination of a lot of work on particle-based transport and erosion systems. Most importantly, a new layered terrain data structure allows for modeling surface and sub-surface phenomena based on particle transport, which exceeds the functionality of voxels.

SoilMachine is written from scratch in very little code and visualized with [TinyEngine](https://github.com/weigert/TinyEngine) using a technique called vertex pooling, which obviates the need to remesh the heightmap (ever). It is designed to be modular and expandable.

This repo is currently in development and will probably not be done soon but I am publishing it so updates can be followed live for anybody who is interested.

### Screenshots



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
