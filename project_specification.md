# Project Specification

## Name
DeferredNerderer <-- WORK IN PROGRESS

## Participants

* Hans-Filip Elo, hanel742
* Lage Ragnarsson, lagra033
* Isak Wiberg, isawi527

## Description

The project will result in a tech demo. The tech demo will illustrate the deffered renderer, preferably with many objects and light sources. The demo will be written in C++, and implemented using at least some parts of the following libraries: 

* SDL2 (https://www.libsdl.org/)
* glM (http://glm.g-truc.net/0.9.7/index.html).
* GLEW (http://glew.sourceforge.net/)
* Assimp (http://www.assimp.org/)

There will be no game-like content and there will also not be any form of editor or sandbox to generate environments.

### Will do features

* Deferred shading/rendering
* Screen space ambient occlusion (SSAO) using G-buffer from deffered renderer
* Movement of the 3D camera both manually and "by rails"
* Multi-mesh model loading

A scene will be created using these techniques. The scene will contain geometry along with many models and light sources.

### Might do features

* Normal mapping
* AA
* Frustum culling
* Gamma correction
* HDR
* Bloom
* Shadow mapping
* Shadow volumes
* Screen Space Reflections

