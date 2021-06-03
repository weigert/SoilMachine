# LayeredErosion

Next iteration of an advanced, modular, coupled geomorphological simulation for erosive surface and sub-surface phenomena.

It should combine features of my previous erosion systems into a single, real-time procedural terrain generator in a modular fashion with low complexity but high realism.

Most significantly, this system introduces a new concept to bring the simulations from 2.5D (i.e. 2D heightmaps) to 2.99D! This opens up many new possibilities without significantly more complexity.

The end goal is to have a single real-time terrain generator that combines all of my previous concepts. This system is a first step towards that by providing a framework for simulating the surface phenomena, based on precipitation data **as an input and** an initial terrain.

This repo is currently in development and will probably not be done soon but I am publishing it so updates can be followed live for anybody who is interested.

## Planned Features

### Vertex Pooling

New rendering technique inspired by voxel engines / AZDO techniques using persistently mapped buffers to avoid terrain remeshing. Should allow for super fast visualization.

### From 2.5D to 2.99D Terrain

New layered-terrain data structure using memory pooling. Inspired by run-length encoding of voxel worlds, but continuous and non-discrete. Should allow for more detailed simulation and **almost** 3D phenomena (including e.g. ovehangs), but without having to resort to discrete voxels, which are more memory intensive and less flexible.

### Coupled Modular Erosion

First experiments with particle parallelization / lagrangian conservative transport phenomena, allowing for coupling hydrological erosion with wind erosion on a single height-map.

## Future Directions

This first experiment, once polished, should then be able to open up new geomorphological possibilities such as:

- Cave formation simulation
- Snow compaction
- Sediment compaction and rock layer formation
- Subterranean flow and waterlogging

without significantly more complexity from previous systems.
