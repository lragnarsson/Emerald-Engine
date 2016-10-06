# Project Specification

## Name
EmeraldEngine

## Participants

* Hans-Filip Elo, hanel742
* Lage Ragnarsson, lagra033
* Isak Wiberg, isawi527

## Description

The project will result in a tech demo. The tech demo will illustrate an open world with procedurally generated grass and shadow mapping. The demo will be written in C++ with OpenGL 3.2+, and implemented using at least some parts of the following libraries:

* SDL2 (https://www.libsdl.org/)
* GLM (http://glm.g-truc.net/0.9.7/index.html).
* GLEW (http://glew.sourceforge.net/)
* Assimp (http://www.assimp.org/)
* EmeraldEngine, this project

There will be no game-like content and there will also not be any form of editor or sandbox to generate environments.


### Will do features

###### Terrain Generation From a Heightmap
By loading an image or generating one at startup.
###### Proceedurally Generated Grass
Grass generated in the geometry shader stage of rendering. The grass should be generated with some form of distance-based level of detail.
###### Grass Animation
Using a gradient map (normal map) such as the ones used for water waves.
###### Directional Light Source with Shadows
A sun which casts shadows using shadow maps
###### Performance optimizations of Emerald
In order to run this demo on our laptops, we need to do some optimizations to EmeraldEngine, as well as be able to disable GPU-intensive features at run time. There are a few obvious things which can be investigated including:

* View space shading
* Light culling on the GPU
* More efficient light source updating
* LOD for models

A scene will be created using these techniques. The scene will contain terrain geometry, along with som models.

### Might do features

###### Shadow Maps for Point Lights
Using cubemaps with depth information for each light source

###### Different level of detail algorithms
For both terrain and grass.

