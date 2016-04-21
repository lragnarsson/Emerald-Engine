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
    /path/to/model rotX rotY rotZ Xpos Ypos Zpos nr_of_lights
  5. Lights are placed DIRECTLY AFTER EACH MODEL and looks like:
    relativeXpos relativeYpos relativeZpos R G B
    DO NOT PLACE COMMENTS BETWEEN MODEL AND ITS LIGHTS
  6. A line in the file needs to be at least as long as the shortest poosible line according to the specification, otherwise it's considered empty.

  Example:
  ----------
  # This is a comment

  # The line below starts a section. Available sections are lights, models, and ground.

  [models]
  /path/to/model Xpos Ypos Zpos rotX rotY rotZ nrOfLights
  relativeXpos relativeYpos relativeZpos R G B

  [flat]
  /path/to/model Xpos Ypos Zpos rotX rotY rotZ nrOfLights
  relativeXpos relativeYpos relativeZpos R G B
  ----------

  The minimum allowed line length is derrived from the shortest line that fulfill the specification. In it's current state that is, for a model section:

  [models]

*/

void load_scene(std::string filepath);

#endif
