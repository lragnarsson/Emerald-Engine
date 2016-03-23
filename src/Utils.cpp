#include "Utils.hpp"

/* Loads, compiles and activates vertex and fragment shaders */
GLuint loadShaders(const GLchar* vertexFilePath, const GLchar* fragmentFilePath)
{
    std::string vertexSource;
    std::string fragmentSource;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::badbit);

    /* Read shaders from files */
    try {
        vShaderFile.open(vertexFilePath);
        fShaderFile.open(fragmentFilePath);

        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        vShaderFile.close();
        fShaderFile.close();

        vertexSource = vShaderStream.str();
        fragmentSource = fShaderStream.str();
    }
    catch (std::ifstream::failure e) {
        std::cerr << "ERROR: Could not read shader files." << std::endl;
    }

    const GLchar* vSource = vertexSource.c_str();
    const GLchar* fSource = fragmentSource.c_str();

    GLuint vertex, fragment;
    GLint success;
    GLchar infoLog[512];

    /* Compile vertex shader */
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vSource, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "ERROR: Vertex shader compilation failed: \n" << infoLog << std::endl;
    }

    /* Compile fragment shader */
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fSource, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "ERROR: Fragment shader compilation failed: \n" << infoLog << std::endl;
    }

    /* Create shader program */
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "ERROR: Shader program linking failed: \n" << infoLog << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    glUseProgram(program);

    return program;
}
