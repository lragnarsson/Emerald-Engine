#ifndef LOADER_H
#define LOADER_H

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <glm/glm.hpp>

#include "Error.hpp"
#include "Light.hpp"
#include "Model.hpp"

/*
  Loader file format specification

  1. Comment line, has to start with hash, in-line comments is not available.
  2. Every type of object to be loaded is in a section consisting of only that type of object.
    - A section is started with "[SECTION]"
  4. Models are loaded according to:
    /path/to/model rotX rotY rotZ Xpos Ypos Zpos
  5. Lights are loaded according to:
    R G B Xpos Ypos Zpos directional/positional
  6. A line in the file needs to be at least as long as the shortest poosible line according to the specification, otherwise it's considered empty.

  Example:
  ----------
  # This is a comment

  # The line below starts a section. Available sections are lights, models, and ground.
  [lights]
  R G B Xpos Ypos Zpos directional/positional

  [models]
  /path/to/model Xpos Ypos Zpos rotX rotY rotZ nrOfLights
  ----------

  The minimum allowed line length is derrived from the shortest line that fulfill the specification. In it's current state that is, for a model section:

  [models]

*/

void load_scene(std::string filepath);

#endif
