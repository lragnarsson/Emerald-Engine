# Project Specification

## Name
DeferredNerderer

## Participants

* Hans-Filip Elo, hanel742
* Lage Ragnarsson, lagra033
* Isak Wiberg, isawi527

## Description

The project will result in a tech demo. The tech demo will illustrate the deffered renderer, preferably with many objects and light sources. The demo will be written in C++ with OpenGL 3.2+, and implemented using at least some parts of the following libraries:

* SDL2 (https://www.libsdl.org/)
* GLM (http://glm.g-truc.net/0.9.7/index.html).
* GLEW (http://glew.sourceforge.net/)
* Assimp (http://www.assimp.org/)

There will be no game-like content and there will also not be any form of editor or sandbox to generate environments.


### Will do features

###### Deferred Shading
Split up rendering in two separate passes using textures as a G-buffer.
###### Screen Space Ambient Occlusion (SSAO)
Calculate occlusion using information in the G-buffer.
###### User Controlled Movement of the 3D Camera
Free movement without collision detection using keyboard and mouse.
###### Multi-Mesh Model Loading
Load models consisting of multiple different meshes with different textures and materials.


A scene will be created using these techniques. The scene will contain geometry along with many models and light sources.


### Might do features

* HDR
* Screen Space Reflections
* Camera animation paths
* Normal mapping
* MSAA
* FXAA
* Frustum culling
* Gamma correction
* Bloom
* Shadow mapping
* Shadow volumes
