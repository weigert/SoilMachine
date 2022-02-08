# soilmachine todo

IO:
- Voxel Export (Raw, Octree, RLE)
- Re-Export .soil file after reconfiguration

Interface:
- Create New Soil Types On-The-Fly
- Create New Initial Terrain Layers On-The-Fly
- Re-Seed Map
- Export Maps to .PNG Files
- Import Maps from .PNG Files
- Export Map to Voxel Formats

Options:
- Run without graphical interface (just import / export)

Visualization:
- Add phong lighting options to soil types for better appearance
- Overlay particle density maps for visuals

Multi WaterTable:
- Pressure Based Seepage
- Pressure Based Sideways Seepage (Map Iterator For Sections)
- Water Cascades to a Particle
- Water Particles Initiate Seepage Too (For General Water Distribution)
-

Multi-Watertable Allows for Vegetation Distributions.

Water cascading to a particle also makes sure that the walls of lakes erode appropriately.

Then: Either multithread using a quadtree OR something else, then make a large world
which contains both a desert and a mountainy area.

Add world dimension to the soil file.

For Caves / Canyons:
- Map needs to be able to "remove" a section below
- There needs to be "collapsing", i.e. subsurface cascading
- When rivers erode a side, they have to actually erode not from the top but at
the level they are at, with the rest collapsing in.

Note: I probably have to adjust the vertexpool meshing to support multiple levels.
This means that every guy needs to be its own triangle which I can add or remove.
