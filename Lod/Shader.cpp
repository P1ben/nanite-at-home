#include "Shader.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

uint32_t Shader::LAST_PROGRAM_LOADED = 0;

Shader::Shader(const char* vertex_path, const char* fragment_path) {
    ReadVertexShader(vertex_path);
    ReadFragmentShader(fragment_path);
    Compile();
}

void Shader::ReadVertexShader(const char* vertex_path) {
    std::ifstream vShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertex_path);
        std::stringstream vShaderStream;

        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();

        // convert stream into string
        vertexCode = vShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::VERTEX::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
}

void Shader::ReadFragmentShader(const char* fragment_path) {
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        fShaderFile.open(fragment_path);
        std::stringstream fShaderStream;

        // read file's buffer contents into streams
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        fShaderFile.close();

        // convert stream into string
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FRAGMENT::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
}

void Shader::Compile() {
    unsigned int vertex, fragment = 0;
    int success;
    char infoLog[512];

    const char* vertex_code = vertexCode.empty() ? nullptr : vertexCode.c_str();
    const char* fragment_code = fragmentCode.empty() ? nullptr : fragmentCode.c_str();

    // Compile vertex shader if exists
    if (vertex_code) {
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vertex_code, NULL);
        glCompileShader(vertex);
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertex, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        };
    }

    // Compile fragment shader if exists
    if (fragment_code) {
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fragment_code, NULL);
        glCompileShader(fragment);
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragment, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        };
    }

    // Linking program
    Id = glCreateProgram();
    if (vertex_code)   glAttachShader(Id, vertex);
    if (fragment_code) glAttachShader(Id, fragment);
    glLinkProgram(Id);
    glGetProgramiv(Id, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(Id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Delete shaders from gpu memory
    if (vertex_code)   glDeleteShader(vertex);
    if (fragment_code) glDeleteShader(fragment);

    GLuint blockIndex = glGetUniformBlockIndex(Id, "Object");

    std::cout << "diskParameters is a uniform block occupying block index: " << blockIndex << '\n';

    GLint blockSize;
    glGetActiveUniformBlockiv(Id, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    std::cout << "\ttaking up " << blockSize << " bytes\n";

    GLubyte* blockBuffer = (GLubyte*)malloc(blockSize);

    const GLchar* names[] = { "modelMatrix","modelMatrixInverse", "drawColor", "useTrueColor"};

    GLuint indices[4];
    glGetUniformIndices(Id, 4, names, indices);

    for (int i = 0; i < 4; ++i) {
        std::cout << "attribute \"" << names[i] << "\" has index: " << indices[i] << " in the block.\n";
    }

    GLint offset[4];
    glGetActiveUniformsiv(Id, 4, indices, GL_UNIFORM_OFFSET, offset);

    for (int i = 0; i < 4; ++i) {
        std::cout << "attribute \"" << names[i] << "\" has offset: " << offset[i] << " in the block.\n";
    }
}

void Shader::Activate() {
    if (Id != LAST_PROGRAM_LOADED) {
        glUseProgram(Id);
        LAST_PROGRAM_LOADED = Id;
    }
}

// Uniform Setters
void Shader::SetBool(Uniform& uniform) {
    glUniform1i(glGetUniformLocation(Id, uniform.name.c_str()), uniform.val_bool);
}
void Shader::SetInt(Uniform& uniform) {
    glUniform1i(glGetUniformLocation(Id, uniform.name.c_str()), uniform.val_int);
}
void Shader::SetFloat(Uniform& uniform) {
    glUniform1i(glGetUniformLocation(Id, uniform.name.c_str()), uniform.val_float);
}
void Shader::SetVec3(Uniform& uniform) {
    glUniform3fv(glGetUniformLocation(Id, uniform.name.c_str()), 1, &uniform.val_vec3.x);
}
void Shader::SetMat4(Uniform& uniform) {
    glUniformMatrix4fv(glGetUniformLocation(Id, uniform.name.c_str()), 1, GL_TRUE, uniform.val_mat4);
}