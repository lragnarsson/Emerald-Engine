#include "Loader.hpp"

using namespace std;

// This defines scene file format
const char _COMMENT_ = '#';
const char _SECTION_STARTER_ =  '[';
const char _SECTION_END_ = ']';
const char _SEPARATOR_ = ' ';
const char _INVISIBLE_CHAR_ = '|';
const unsigned int _MINIMUM_ALLOWED_LINE_LENGTH_ = 6;

const string _TERRAIN_ = "[terrain]";
const string _MODELS_ = "[models]";
const string _LIGHTS_ = "[lights]";
const string _FLAT_ = "[flat]";
const string _ANIMATIONS_ = "[animations]";
const string _CAMERA_ANIMATION_ = "[camera_animation]";

// ------------------

vector<string> Loader::split_string(string input, char separator, char inv_char)
{
    vector<string> tokens;
    size_t start = 0, end = 0;

    // Remove all occurences of the "invisible char"
    input.erase(remove(input.begin(), input.end(), inv_char), input.end());
    // If double spaces, make them into one
    input = regex_replace(input, regex("  "), " ");

    while ((end = input.find(separator, start)) != string::npos) {
        string temp = input.substr(start, end - start);
        for (int i = temp.size()-1; i >= 0; i--){
            if (temp.at(i) == ' '){
                temp.erase(temp.begin()+i);
            }
        }
        if (temp != "") tokens.push_back(temp);
        start = end + 1;
    }

    string temp = input.substr(start);
    if (temp != "") tokens.push_back(temp);

    return tokens;
}

// ------------------
// Attaches default spline to camera:
// idOfFollow startPoint | idLookAt startPoint

void Loader::init_camera_anim_path(std::vector<std::string> camera_line, Camera* camera)
{
    std::vector<float> numbers;

    for (size_t i = 0; i < camera_line.size(); i++) {
        numbers.push_back(stof(camera_line[i]));
    }

    int move_id = (int)numbers[0];
    int look_id = (int)numbers[2];

    if ( move_id != -1 && look_id != -1) {
        camera->attach_move_animation_path(move_id, numbers[1]);
        camera->attach_look_animation_path(look_id, numbers[3]);
    }
    else{
        printf("WARNING, no animation path attached. Move_id: %i, Look_id: %i\n", move_id, look_id);
    }
}

// ------------------
// Animation_Path::Animation_Path(vector<glm::vec3> points, float period)

void Loader::load_animation(vector<string> animation_line){
    #ifdef _DEBUG_LOADER_
    int nr_of_points = 0;
    #endif

    float convert_x, convert_y, convert_z, period;
    vector<glm::vec3> points;

    for (size_t i = 0; i < animation_line.size()-1; i += 3) {
        points.push_back(glm::vec3(stof(animation_line[i]), stof(animation_line[i+1]), stof(animation_line[i+2])));

        #ifdef _DEBUG_LOADER_
        nr_of_points++;
        cout << "Added point: {" << stof(animation_line[i]) << "," << stof(animation_line[i+1]) << "," << stof(animation_line[i+2]) << "}" << endl;
        cout << "Nr of points in animation path is now: " << nr_of_points << endl;
        #endif
    }
    // Period needs to be a number as well
    period = stof(animation_line.back());

    if (points.size() > 3) {
        new Animation_Path(points, period);
        return;
    } else {
        #ifdef _DEBUG_LOADER_
        cout << "Too few points in animation path. No object created." << endl;
        #endif
        Error::throw_error(Error::invalid_file_syntax,
                           string("Too few points in animation path! "));
    }
}

// -----------------

void Loader::load_terrain(vector<string> terrain_line){
#ifdef _DEBUG_LOADER_
    for (int i = 0; i < terrain_line.size(); i++){
        cout << terrain_line.at(i) << " ";
    }
    cout << endl;
#endif
    string directory = terrain_line.at(0);
    float planar_scale = stof(terrain_line.at(1));
    float height_scale = stof(terrain_line.at(2));
    new Terrain(directory, planar_scale, height_scale);
}

// ------------------

Light *Loader::load_light(vector<string> light_line)
{
    vector<float> numbers;

    for (size_t i = 0; i < light_line.size(); i++) {
        numbers.push_back(stof(light_line[i]));
    }

    return new Light(glm::vec3(numbers[0], numbers[1], numbers[2]), 1.f, glm::vec3(numbers[3], numbers[4], numbers[5]));
}

// ------------------

void Loader::load_model(ifstream* read_file, int* current_line, vector<string>& model_line, bool flat)
{
    // /path/to/model Xpos Ypos Zpos rotX rotY rotZ nrOfLights
    vector<float> numbers;

    string light_line;
    int animation_id = -1;
    float animation_start_point;

    int nr_of_lights;
    Light *attach_light;
    Model* this_model;


    for (size_t i = 1; i < model_line.size()-2; i++) {
        numbers.push_back(stof(model_line[i]));
    }
    animation_id = stoi(model_line.at(model_line.size()-3));
    animation_start_point = stof(model_line.at(model_line.size()-2));
    nr_of_lights = stof(model_line.back());

    // Create rotational matrix for model.
    glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), -numbers[0], glm::vec3(1.f, 0.f, 0.f));
    glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), -numbers[1], glm::vec3(0.f, 1.f, 0.f));
    glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), numbers[2], glm::vec3(0.f, 0.f, 1.f));

    glm::mat4 total_rot = rotX * rotY * rotZ;

    // Create model
    this_model = new Model(model_line[0], total_rot, glm::vec3(numbers[3], numbers[4], numbers[5]), numbers[6], flat);

    // Attach animation path if any:
    if (animation_id != -1) {
        #ifdef _DEBUG_LOADER_
        cout << "Attaching animation path with id " << animation_id << " and startpoint " << animation_start_point << endl;
        #endif
        this_model->attach_animation_path(animation_id, animation_start_point);
    }

    // Attach light sources to model
    if (nr_of_lights > 0) {
        for (size_t i = 0; i < nr_of_lights; i++) {
            getline(*read_file,light_line);

            #ifdef _DEBUG_LOADER_
            *current_line++;
            cout << "Light source attached to model: " << light_line << endl;
            #endif

            attach_light = load_light(split_string(light_line, _SEPARATOR_, _INVISIBLE_CHAR_) );
            this_model->attach_light(attach_light,
                                     attach_light->position);
        }
    }
}

// ------------------

void Loader::load_scene(string filepath, Camera* camera)
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
        Error::throw_error(Error::cant_open_world_file, filepath);
    }

    while ( getline(read_file,line) ) {
        current_line++;

        #ifdef _DEBUG_LOADER_
        cout << endl;
        cout << "Current line: " << current_line << endl;
        #endif

        if ( line.length() < _MINIMUM_ALLOWED_LINE_LENGTH_ ) {
            // This line is assumed empty, see file format specification in Loader.hpp
            #ifdef _DEBUG_LOADER_
            cout << "Line " << current_line << " too short: " << endl;
            #endif
            continue;
        }

        // Split string on single space
        split_line = split_string(line, _SEPARATOR_, _INVISIBLE_CHAR_);
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
            if (split_line.at(0) == _TERRAIN_){
                #ifdef _DEBUG_LOADER_
                cout << "Line " << current_line << " starts a terrain section " << endl;
                #endif
                current_section = _TERRAIN_;
            }
            else if (split_line.at(0) == _MODELS_){
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
            else if ( split_line.at(0) == _ANIMATIONS_ ){
                #ifdef _DEBUG_LOADER_
                cout << "Line " << current_line << " starts an animation section " << endl;
                #endif
                current_section = _ANIMATIONS_;
            }
            else if ( split_line.at(0) == _LIGHTS_ ){
                #ifdef _DEBUG_LOADER_
                cout << "Line " << current_line << " starts a lights section " << endl;
                #endif
                current_section = _LIGHTS_;
            }
            else if ( split_line.at(0) == _CAMERA_ANIMATION_ ){
                #ifdef _DEBUG_LOADER_
                cout << "Line " << current_line << " starts a lights section " << endl;
                #endif
                current_section = _CAMERA_ANIMATION_;
            }
            else {
                Error::throw_error(Error::invalid_file_syntax, "On line " + to_string(current_line));
            }
            continue;
        }

        // Load the type that is needed
        if (current_section == _TERRAIN_){
            #ifdef _DEBUG_LOADER_
            cout << "Loading terrain!" << endl;
            cout << line << endl;
            #endif
            load_terrain(split_line);
            continue;
        }
        else if (current_section == _MODELS_) {
            #ifdef _DEBUG_LOADER_
            cout << "Loading model!" << endl;
            cout << line << endl;
            #endif
            load_model(&read_file, &current_line, split_line, false);
            continue;
        }
        else if (current_section == _FLAT_){
            #ifdef _DEBUG_LOADER_
            cout << "Loading flat shaded model!" << endl;
            cout << line << endl;
            #endif
            load_model(&read_file, &current_line, split_line, true);
            continue;
        }
        else if (current_section == _ANIMATIONS_){
            #ifdef _DEBUG_LOADER_
            cout << "Loading animation path!" << endl;
            cout << line << endl;
            #endif
            load_animation(split_line);
            continue;
        }
        else if (current_section == _LIGHTS_){
            #ifdef _DEBUG_LOADER_
            cout << "Loading light without model!" << endl;
            cout << line << endl;
            #endif
            load_light(split_line);
            continue;
        }
        else if (current_section == _CAMERA_ANIMATION_){
            #ifdef _DEBUG_LOADER_
            cout << "Loading light without model!" << endl;
            cout << line << endl;
            #endif
            init_camera_anim_path(split_line, camera);
            continue;
        }

        Error::throw_error(Error::invalid_file_syntax, "On line " + to_string(current_line));
    }
}
