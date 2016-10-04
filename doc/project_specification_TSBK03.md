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

###### Proceedurally generated grass
Grass generated in geometry shader stage of rendering. The grass should be generated with some form of distance-based level of detail.
###### Shadow mapping
Render scene with shadow mapping. Each moving light source needs to be re-rendered ''live''.
###### Performance optimizations of Emerald
In order to run this demo on our laptops, we need to do some optimizations to EmeraldEngine, as well as be able to disable GPU-intensive features dynamically. 

A scene will be created using these techniques. The scene will contain a base geometry, along with som models.

### Might do features

* Different movement patterns for procedually generated grass
* Different level of detail algorithms

