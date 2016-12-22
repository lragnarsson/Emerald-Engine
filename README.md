Emerald Engine
--------
3D graphics engine written in C++ using OpenGL. This project was first created as part of a course
in computer graphics at Linköping University. Grass, shadows, bloom and more was added as in a
later course.

Many parts of this project have been based on tutorials and examples found online, [learnopengl.com](http://www.learnopengl.com) in particular has been a good source of inspiration. The grass was
inspired by this [blog post](http://outerra.blogspot.se/2012/05/procedural-grass-rendering.html).

Project members:
 - Hans-Filip Elo
 - Lage Ragnarsson
 - Isak Wiberg

A video showcasing the procedural grass:
https://www.youtube.com/watch?v=xNea72F_aws

An older video:
https://www.youtube.com/watch?v=Tt9o01yyQiw

FEATURES
--------
 - Deferred shading
 - Screen Space Ambient Occlusion
 - Shadow maps
 - Procedural grass with LOD
 - Grass animation based on wind and collisions
 - Bloom
 - Scene loader
 - Terrain generation
 - Catmull-Rom splines for object and camera animation
 - Frustum culling for meshes and lights
 - Normal- and specular maps
 - Shader precompiler
 - Day-night cycle
 - Switch between different rendering paths during runtime:
   1. Forward
   2. Deferred
   3. Positions
   4. Normals
   5. Albedo
   6. Specular intensity
   7. SSAO buffer
   8. Shadow map
 - Keyboard and mouse control
 - Bounding sphere generation and visualization
 - Normal vector visualization
 - Light sources attached to models


INSTALLATION
--------

### Debian / Ubuntu
Install dependencies with apt-get:

```shell
sudo apt-get install build-essential libgl1-mesa-dev libsdl2-dev libglew-dev libglm-dev libsdl2-image-dev libassimp-dev
```

Go to the [AntTweakBar download page](http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:download) and download the zip file. Extract the folder and move inside its src/ folder. Run make. Move the contents of /include to /usr/local/include and the contents of lib to /usr/local/lib. run
```shell
sudo ldconfig
```

Run make

### MAC OS X
Install dependencies with brew:

```shell
brew install glm assimp sdl2 sdl2_image anttweakbar
```

Run make
