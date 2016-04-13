#include "Loader.hpp"

using namespace std;

// This defines scene file format
const char _COMMENT_ = '#';
const char _SECTION_STARTER_ =  '[';
const char _SECTION_END_ = ']';

const string _MODELS_ = "[models]";
const string _LIGHTS_ = "[lights]";

const char _SEPARATOR_ = ' ';
const unsigned int _MINIMUM_ALLOWED_LINE_LENGTH_ = 8;

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
bool load_light(vector<string>& light_line)
{
  // This is dummy so far.
  //new Light(glm::vec3(), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.f));
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
    if ( line.length() < _MINIMUM_ALLOWED_LINE_LENGTH_ ) {
      // This line is assumed empty, see file format specification in Loader.hpp
      continue;
    }

    split_line = split_string(line, _SEPARATOR_);
    first_char = split_line.at(0).at(0);

    // Check for comment
    if (first_char == _COMMENT_) {
      continue;
    }

    // Check for a section
    if ( first_char == _SECTION_STARTER_) {
      if (split_line.at(0) == _MODELS_){
        cout << "Models not implemented yet." << endl;
        current_section = _MODELS_;
      }
      else if (split_line.at(0) == _LIGHTS_) {
        current_section = _LIGHTS_;
      }
      else {
        Error::throw_error(Error::invalid_file_syntax, "On line " + to_string(current_line));
      }
      continue;
    }

    // Load the type that is needed
    if (current_section == _LIGHTS_) {
      load_light(split_line);
      continue;
    }
    else if (current_section == _MODELS_){
      continue;
    }

    Error::throw_error(Error::invalid_file_syntax, "On line " + to_string(current_line));
  }
}
