#include "Shader.h"

#include <cstring>
#include <cstdio>

#include "Util.h"

Shader::Shader(const char* vertexName, const char* fragmentName)
{
    char vertexFullName[32] = "res/";
    char fragmentFullName[32] = "res/";

    strcat(vertexFullName, vertexName); strcat(vertexFullName, ".vert");
    strcat(fragmentFullName, fragmentName); strcat(fragmentFullName, ".frag");

    FILE* vShaderFile;
    FILE* fShaderFile;
    
    // read file contents with jank C functions (owwww)
    vShaderFile = fopen(vertexFullName, "r");
    if(!vShaderFile) {
        printf("Failed to read vertex shader \"%s\".\n", vertexFullName);
        return;
    }

    fseek(vShaderFile, 0L, SEEK_END);
    int vShaderSize = ftell(vShaderFile);
    fseek(vShaderFile, 0L, SEEK_SET);

    char* vertexSource = new char[vShaderSize + 1];
    for(int pos = 0; pos < vShaderSize; pos++)
    {
        vertexSource[pos] = getc(vShaderFile);
    }
    vertexSource[vShaderSize] = '\0'; // null-terminate 

    fclose(vShaderFile);

    fShaderFile = fopen(fragmentFullName, "r");
    if(!fShaderFile) {
        printf("Failed to read fragment shader \"%s\".\n", fragmentFullName);
        return;
    }

    fseek(fShaderFile, 0L, SEEK_END);
    int fShaderSize = ftell(fShaderFile);
    fseek(fShaderFile, 0L, SEEK_SET);

    char* fragmentSource = new char[fShaderSize + 1];
    for(int pos = 0; pos < fShaderSize; pos++)
    {
        fragmentSource[pos] = getc(fShaderFile);
    }
    fragmentSource[fShaderSize] = '\0'; // null-terminate 

    fclose(fShaderFile);


    // compile
    GLuint vertex, fragment;
    int success;
    char infoLog[512];

    
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, nullptr);
    glCompileShader(vertex);
    
    // catch compile errors
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        printf("Failed to compile vertex shader \"%s\". Error log:\n%s\n\n", vertexFullName, infoLog);
        return;
    }
    
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, nullptr);
    glCompileShader(fragment);
    
    // catch compile errors
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertex, 512, nullptr, infoLog);
        printf("Failed to compile fragment shader \"%s\". Error log:\n%s\n\n", fragmentFullName, infoLog);
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
        printf("Failed to link shader \"%s\" & \"%s\"! Error log:\n%s\n\n", vertexFullName, fragmentFullName, infoLog);
        return;
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    cacheUniforms();
}

Shader::Shader(const char* computeName, bool hasExtra, const char* extraCode)
{
    int extraLength = strlen(extraCode);

    char computeFullName[32] = "res/";
    strcat(computeFullName, computeName); strcat(computeFullName, ".comp");

    FILE* computeFile;
    computeFile = fopen(computeFullName, "r");
    if(!computeFile) {
        printf("Failed to read compute shader \"%s\".\n", computeFullName);
        return;
    }

    fseek(computeFile, 0L, SEEK_END);
    int cShaderFileSize = ftell(computeFile);
    fseek(computeFile, 0L, SEEK_SET);

    // also allocate space for extraCode
    char* computeSource = new char[cShaderFileSize + extraLength + 1]; // for null terminate
    for(int pos = 0; pos < cShaderFileSize; pos++)
    {
        computeSource[pos] = getc(computeFile);
    }
    computeSource[cShaderFileSize] = '\0'; // null-terminate 

    fclose(computeFile);

    if(hasExtra)
    { // augh gotta insert into C string
        int startLength = strlen("#version 430\n");
        int tailLength = cShaderFileSize + 1 - startLength;

        char* tail = new char[tailLength];
        strcpy(tail, computeSource + startLength);

        strcpy(computeSource + startLength, extraCode);

        strcpy(computeSource + startLength + extraLength, tail);
    }

    computeSource[cShaderFileSize + extraLength] = '\0'; // null-terminate jic

    const GLuint computeShader = glCreateShader(GL_COMPUTE_SHADER);

    glShaderSource(computeShader, 1, &computeSource, nullptr);
    glCompileShader(computeShader);

    // catch errors
    int success;
    char infoLog[512];

    glGetShaderiv(computeShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(computeShader, 512, nullptr, infoLog);
        printf("Failed to compile compute shader \"%s\"! Error log:\n%s\n\n", computeFullName, infoLog);
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
        printf("Failed to link compute shader \"%s\"! Error log:\n%s\n\n", computeFullName, infoLog);
        return;
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

    printf("ERROR: Can't find uniform %s\n", uniformName);
    return 0;
}