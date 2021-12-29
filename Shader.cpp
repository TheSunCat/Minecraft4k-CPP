#include "Shader.h"

#include <cstring>

#include "Util.h"

Shader::Shader(const char* vertexCode, const char* fragmentCode)
{
    // compile
    GLuint vertex, fragment;
    int success;
    char infoLog[512];

    
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexCode, nullptr);
    glCompileShader(vertex);
    
    // catch compile errors
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        prints("\n\nERROR: Failed to compile vertex shader! Error log:\n"); prints(infoLog); prints("\n\n");
        crash();
    }
    
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentCode, nullptr);
    glCompileShader(fragment);
    
    // catch compile errors
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        prints("\n\nERROR: Failed to compile fragment shader! Error log:\n"); prints(infoLog); prints("\n\n");
        crash();
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
        prints("\n\nERROR: Failed to link shader program! Error log:\n"); prints(infoLog); prints("\n\n");
        crash();
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    cacheUniforms();
}

Shader::Shader(const char* computeCode)
{
    prints(computeCode);

    const GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

    glShaderSource(computeShader, 1, &computeCode, nullptr);
    glCompileShader(computeShader);

    // catch errors
    int success;
    char infoLog[512];

    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(computeShader, 512, nullptr, infoLog);
        prints("\n\nERROR: Failed to compile compute shader! Error log:\n"); prints(infoLog); prints("\n\n");
        crash();
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
        prints("\n\nERROR: Failed to link compute shader! Error log:\n"); prints(infoLog); prints("\n\n");
        crash();
    }

    glDetachShader(ID, computeShader);
    glDeleteShader(computeShader);

    cacheUniforms();
}

// use/activate the shader
void Shader::use() const {
    glUseProgram(ID);
}

// utility uniform functions
void Shader::setBool(const char* name, int len, const bool value) const {
    glUniform1i(getUniformLocation(name, len), int(value));
}
// ------------------------------------------------------------------------
void Shader::setInt(const char* name, int len, const int value) const {
    glUniform1i(getUniformLocation(name, len), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const char* name, int len, const float value) const {
    glUniform1f(getUniformLocation(name, len), value);
}
// ------------------------------------------------------------------------
void Shader::setVec2(const char* name, int len, const vec2& value) const
{
    glUniform2f(getUniformLocation(name, len), value.x, value.y);
}
void Shader::setVec2(const char* name, int len, const float x, const float y) const
{
    glUniform2f(getUniformLocation(name, len), x, y);
}
// ------------------------------------------------------------------------
void Shader::setVec3(const char* name, int len, const vec3& value) const
{
    glUniform3f(getUniformLocation(name, len), value.x, value.y, value.z);
}
void Shader::setVec3(const char* name, int len, const float x, const float y, const float z) const
{
    glUniform3f(getUniformLocation(name, len), x, y, z);
}

void Shader::cacheUniforms()
{
    GLint i;
    GLint count; // uniform count

    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei bufSize = 16; // maximum name length
    GLchar name[bufSize]; // variable name in GLSL
    GLsizei length; // name length

    glGetProgramiv(ID, GL_ACTIVE_UNIFORMS, &count);
    //uniformCache.reserve(count);

    for (i = 0; i < count; i++)
    {
        glGetActiveUniform(ID, (GLuint)i, bufSize, &length, &size, &type, name);

        uniformCache.push_back(murmurHash2(name, length));
    }
}

GLint Shader::getUniformLocation(const char* uniformName, int len) const
{
    unsigned int hash = murmurHash2(uniformName, len);

    int loc = 0;

    for(int loc = 0; loc < uniformCache.size(); loc++)
    {
        if(uniformCache[loc] == hash)
            return loc;
    }

    prints("ERROR: Can't find uniform "); prints(uniformName); prints("\n");
    return 0;
}