#include "Loader.hpp"

using namespace std;

// This defines scene file format
const char _COMMENT_ = '#';
const char _SECTION_STARTER_ =  '[';
const char _SECTION_END_ = ']';

const string _MODELS_ = "[models]";
const string _LIGHTS_ = "[lights]";
const string _FLAT_ = "[flat]";

const char _SEPARATOR_ = ' ';
const unsigned int _MINIMUM_ALLOWED_LINE_LENGTH_ = 6;

// ------------------

vector<string> split_string(string input, char separator)
{
  vector<string> tokens;
  size_t start = 0, end = 0;

  while ((end = input.find(separator, start)) != string::npos) {
    string temp = input.substr(start, end - start);
    if (temp != "") tokens.push_back(temp);
    start = end + 1;
  }

  string temp = input.substr(start);
  if (temp != "") tokens.push_back(temp);

  return tokens;
}

// ------------------
// Light::Light(const glm::vec3 world_coord, const glm::vec3 ambient_color,const glm::vec3 diffuse_color, const glm::vec3 specular_color)
Light* load_light(vector<string> light_line)
{
  float converter;
  vector<float> numbers;

  for (size_t i = 0; i < light_line.size(); i++) {
    stringstream(light_line[i]) >> converter;
    numbers.push_back(converter);
  }

  return new Light(glm::vec3(numbers[0], numbers[1], numbers[2]), glm::vec3(numbers[3], numbers[4], numbers[5]));
}

// ------------------

bool load_model(ifstream* read_file, int* current_line, vector<string>& model_line, bool flat)
{
  // /path/to/model Xpos Ypos Zpos rotX rotY rotZ nrOfLights
  vector<float> numbers;
  float converter;

  string light_line;
  int nr_of_lights;
  Light* attach_light;
  Model* this_model;

  for (size_t i = 1; i < model_line.size()-1; i++) {
    stringstream(model_line[i]) >> converter;
    numbers.push_back(converter);
  }
  stringstream(model_line.back()) >> nr_of_lights;

  // Create model
  this_model = new Model(model_line[0], glm::mat4(1.f), glm::vec3(numbers[3], numbers[4], numbers[5]), flat);

  // Attach light sources to model
  if (nr_of_lights > 0) {
    for (size_t i = 0; i < nr_of_lights; i++) {
      *current_line++;
      getline(*read_file,light_line);
      attach_light = load_light( split_string(light_line, _SEPARATOR_) );
      this_model->attach_light(attach_light, attach_light->get_pos());
    }
  }

  return true;
}

// ------------------

void load_scene(string filepath)
{
  // Input
  ifstream read_file(filepath);
  // Temp variables
  string line;
  vector<string> split_line;
  char first_char;
  // States
  string current_section;
  int current_line = 0;

  if ( !read_file.is_open() ) {
    Error::throw_error(Error::cant_open_world_file);
  }

  while ( getline(read_file,line) ) {
    current_line++;

    #ifdef _DEBUG_LOADER_
    cout << "Current line: " << current_line << endl;
    #endif

    if ( line.length() < _MINIMUM_ALLOWED_LINE_LENGTH_ ) {
      // This line is assumed empty, see file format specification in Loader.hpp
      #ifdef _DEBUG_LOADER_
      cout << "Line " << current_line << " too short: " << endl;
      #endif
      continue;
    }

    split_line = split_string(line, _SEPARATOR_);
    first_char = split_line.at(0).at(0);

    // Check for comment
    if (first_char == _COMMENT_) {
      #ifdef _DEBUG_LOADER_
      cout << "Line " << current_line << " is a comment: " << endl;
      #endif
      continue;
    }

    // Check for a section
    if ( first_char == _SECTION_STARTER_) {
      #ifdef _DEBUG_LOADER_
      cout << "Line " << current_line << " starts with [" << endl;
      #endif
      if (split_line.at(0) == _MODELS_){
        #ifdef _DEBUG_LOADER_
        cout << "Line " << current_line << " starts a model section " << endl;
        #endif
        current_section = _MODELS_;
      }
      else if (split_line.at(0) == _FLAT_) {
        #ifdef _DEBUG_LOADER_
        cout << "Line " << current_line << " starts a flat section: " << endl;
        #endif
        current_section = _FLAT_;
      }
      else {
        Error::throw_error(Error::invalid_file_syntax, "On line " + to_string(current_line));
      }
      continue;
    }

    // Load the type that is needed
    if (current_section == _MODELS_) {
      #ifdef _DEBUG_LOADER_
      cout << "Loading model!" << endl;
      #endif
      load_model(&read_file, &current_line, split_line, false);
      continue;
    }
    else if (current_section == _FLAT_){
      #ifdef _DEBUG_LOADER_
      cout << "Loading flat shaded model!" << endl;
      #endif
      load_model(&read_file, &current_line, split_line, true);
      continue;
    }

    Error::throw_error(Error::invalid_file_syntax, "On line " + to_string(current_line));
  }
}
