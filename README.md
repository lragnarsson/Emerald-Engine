Emerald Engine
--------

Project for the course TSBK07 - Computer Graphics at Linköping University.

Many parts of this project have been based on tutorials and examples found online, [learnopengl.com](http://www.learnopengl.com) in particular has been a good source of inspiration.

Project members:
 - Hans-Filip Elo
 - Lage Ragnarsson
 - Isak Wiberg

A video showing some of the features in Emerald Engine is available here:
https://www.youtube.com/watch?v=Tt9o01yyQiw

FEATURES
--------
 - Deferred shading
 - Screen Space Ambient Occlusion
 - Scene loader
 - Catmull-Rom splines for object and camera animation
 - Frustum culling
 - Normal maps
 - Shader precompiler
 - Switch between different rendering paths during runtime:
   1. Forward
   2. Deferred
   3. Positions
   4. Normals
   5. Albedo
   6. Specular intensity
   7. SSAO buffer
 - Specular maps
 - Skybox
 - Keyboard and mouse control
 - Bounding sphere generation and visualization
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
