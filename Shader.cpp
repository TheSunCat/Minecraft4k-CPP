#include "Shader.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Shader::Shader(std::string vertexName, std::string fragmentName)
{
    vertexName = "res/" + vertexName + ".vert";
    fragmentName = "res/" + fragmentName + ".frag";

    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        std::stringstream vShaderStream, fShaderStream;
        
        // read file buffer contents into streams, then into strings
        vShaderFile.open(vertexName);
        vShaderStream << vShaderFile.rdbuf();
        vShaderFile.close();
        vertexCode = vShaderStream.str();

        fShaderFile.open(fragmentName);
        fShaderStream << fShaderFile.rdbuf();
        fShaderFile.close();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "Failed to read one of the shader files. It's \"" << vertexName << "\" or \"" << fragmentName << "\". No, I'm not telling you which.\n"
                  << "Error code: " << e.code() << std::endl;
        return;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();


    // compile
    GLuint vertex, fragment;
    int success;
    char infoLog[512];

    
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, nullptr);
    glCompileShader(vertex);
    
    // catch compile errors
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cout << "Failed to compile vertex shader \"" << vertexName << "\". Error log:\n" << infoLog << std::endl;
        return;
    }
    
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, nullptr);
    glCompileShader(fragment);
    
    // catch compile errors
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        std::cout << "Failed to compile fragment shader \"" << fragmentName << "\". Error log:\n" << infoLog << std::endl;
        return;
    }

    
    // create program & attach shaders
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);

    glLinkProgram(ID);
    
    // catch linking errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cout << "Failed to link shader \"" << vertexName << "\" & \"" << fragmentName << "\"! Error log:\n" << infoLog << std::endl;
        return;
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::Shader(std::string computeName, HasExtra hasExtra, const char* extraCode)
{
    computeName = "res/" + computeName + ".comp";

    std::string computeCode;
    std::ifstream computeFile;

    computeFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        std::stringstream computeStream;

        computeFile.open(computeName);

        computeStream << computeFile.rdbuf();
        computeFile.close();
        computeCode = computeStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "Failed to load compute shader \"" << computeName << "\"!"
                  << "Error code: " << e.code() << std::endl;
        return;
    }

    if(hasExtra == HasExtra::Yes)
        computeCode.insert(strlen("#version 430\n"), extraCode);

    const GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

    char const* computeSource = computeCode.c_str();
    glShaderSource(computeShader, 1, &computeSource, nullptr);
    glCompileShader(computeShader);

    // catch errors
    int success;
    char infoLog[512];

    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(computeShader, 512, nullptr, infoLog);
        std::cout << "Failed to compile compute shader \"" << computeName << "\"! Error log:\n" << infoLog << std::endl;
        return;
    }

    // link the program
    ID = glCreateProgram();
    glAttachShader(ID, computeShader);
    glLinkProgram(ID);

    // catch errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, nullptr, infoLog);
        std::cout << "Failed to link compute shader \"" << computeName << "\"! Error log:\n" << infoLog << std::endl;
        return;
    }

    glDetachShader(ID, computeShader);
    glDeleteShader(computeShader);
}

Shader::Shader(HasExtra, const std::string source)
{
    const GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

    char const* computeSource = source.c_str();
    glShaderSource(computeShader, 1, &computeSource, nullptr);
    glCompileShader(computeShader);

    // catch errors
    int success;
    char infoLog[512];

    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(computeShader, 512, nullptr, infoLog);
        std::cout << "Failed to compile compute shader \"default\"! Error log:\n" << infoLog << std::endl;
        return;
    }

    // link the program
    ID = glCreateProgram();
    glAttachShader(ID, computeShader);
    glLinkProgram(ID);

    // catch errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(computeShader, 512, nullptr, infoLog);
        std::cout << "Failed to link compute shader \"default\"! Error log:\n" << infoLog << std::endl;
        return;
    }

    glDetachShader(ID, computeShader);
    glDeleteShader(computeShader);
}

// use/activate the shader
void Shader::use() const {
    glUseProgram(ID);
}

// utility uniform functions
void Shader::setBool(const std::string& name, const bool value) const {
    glUniform1i(getUniformLocation(name.c_str()), int(value));
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string& name, const int value) const {
    glUniform1i(getUniformLocation(name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string& name, const float value) const {
    glUniform1f(getUniformLocation(name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(getUniformLocation(name.c_str()), 1, &value[0]);
}
void Shader::setVec2(const std::string& name, const float x, const float y) const
{
    glUniform2f(getUniformLocation(name.c_str()), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    glUniform3fv(getUniformLocation(name.c_str()), 1, &value[0]);
}
void Shader::setVec3(const std::string& name, const float x, const float y, const float z) const
{
    glUniform3f(getUniformLocation(name.c_str()), x, y, z);
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(getUniformLocation(name.c_str()), 1, &value[0]);
}
void Shader::setVec4(const std::string& name, const float x, const float y, const float z, const float w) const
{
    glUniform4f(getUniformLocation(name.c_str()), x, y, z, w);
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string& name, const glm::mat2& mat) const
{
    glUniformMatrix2fv(getUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const
{
    glUniformMatrix3fv(getUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    glUniformMatrix4fv(getUniformLocation(name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

GLint Shader::getUniformLocation(const char* uniformName) const
{
    const auto f = uniformCache.find(uniformName);

    GLint loc;

    if (f == uniformCache.end()) { // get uniform location
        loc = glGetUniformLocation(ID, uniformName);

        std::pair<std::string, GLuint> newLoc(uniformName, loc);
        uniformCache.insert(newLoc);
    }
    else
    {
        loc = (*f).second;
    }

    return loc;
}