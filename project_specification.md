# Project Specification

## Name
DeferredNerderer <-- WORK IN PROGRESS

## Participants

* Lage Ragnarsson, lagra033
* Isak Wiberg, isawi527
* Hans-Filip Elo, hanel742

## Description

The project will result in a tech demo. The tech demo will illustrate the deffered renderer, preferably with many objects and light sources. The project will be implemented using SDL2 (https://www.libsdl.org/) and glM (http://glm.g-truc.net/0.9.7/index.html).

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
