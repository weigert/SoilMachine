# SoilMachine

Advanced, modular, coupled geomorpohology simulator for real-time procedural terrain generation in C++.

Visualization with [TinyEngine](https://github.com/weigert/TinyEngine).

Article: [An Efficient Data Structure for 3D Multi-Layer Terrain and Erosion Simulation](https://nickmcd.me/2022/04/15/soilmachine/)

## Description

SoilMachine is a unified geomorphology simulator for procedural terrain. It represents the culmination of a lot of work on particle-based transport and erosion systems (**hydraulic and wind**). Most importantly, a new **layered terrain** data structure allows for modeling **surface and sub-surface phenomena** based on **particle transport** with **multiple soil types**, which exceeds the functionality of voxels.

![Banner](https://github.com/weigert/SoilMachine/blob/master/screenshots/banner.png)

SoilMachine is written from scratch in very little code and visualized with [TinyEngine](https://github.com/weigert/TinyEngine) using a technique called vertex pooling, which obviates the need to remesh the heightmap (ever). It is designed to be modular and expandable.

This repo is currently in development and will probably not be done soon but I am publishing it so updates can be followed live for anybody who is interested.

## Utilization

Tested on Ubuntu20 LTS. See below for how to build.

    Reading:

      <> optional
      [] required

    Run:

      ./soilmachine <options/flags>

    Options:

      -SEED [#]     Run using sed. No seed = random
      -soil [file]  Specify relative path to .soil file

      -oc [file]    Export color map to .png file at relative path (on program exit)
      -oh [file]    Export height map to .png file at relative path (on program exit)

Note: More options and flags are planned in the future. See the `todo.md` file for more information.

### Controls

    ESC: Toggle GUI
    P: Toggle Pause (Paused by default!!!)

    Scroll-Y: Zoom
    Scroll-X: Camera Angle
    Arrow Keys: Camera Orientation
    C / V: Move Camera Up / Down
    WASD: Move Camera in Plane

In the GUI, a number of things can be done, including Re-Seeding the map, setting erosion parameters, tuning the soil properties without re-launching the program and tuning the visualization. More features are planned in the future (see `todo.md` for more information).

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

## Features

**Implemented**

- Vertexpool based heightmap visualization (OpenGL AZDO) to eliminate remesh cost
- Layered terrain data structure using memory pooled, run-length encoded sediment sections as a linked list on a grid
- Modular soil type description and "erosion chains"
- Modular description of transport types using a particle base class
  - Wind erosion particles based on [Particle-Based Wind Erosion](https://github.com/weigert/SimpleWindErosion)
  - Hydraulic erosion particles based on [Particle-Based Erosion](https://github.com/weigert/SimpleErosion)
- GUI Interface for Initial Terrain
- GUI Interface for Defining Sediment Properties
- Data Export (Using Commandline Flags)

**Planned**

- Parallelization of the particle base class terrain interaction
- Sediment compaction / conversion
- Water-logging using sediment porosity
- Implementation of multiple water tables based on [Particle-Based Hydrology](https://github.com/weigert/SimpleHydrology)

### Screenshots

Some example outputs:

![Output 1](https://github.com/weigert/SoilMachine/blob/master/screenshots/Output1.png)

![Output 2](https://github.com/weigert/SoilMachine/blob/master/screenshots/Output2.png)

![Output 3](https://github.com/weigert/SoilMachine/blob/master/screenshots/Output3.png)

## License

MIT License
