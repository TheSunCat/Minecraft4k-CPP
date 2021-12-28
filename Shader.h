#pragma once

#include <glad/glad.h>
#include <vector>

#include "Vector.h"

class Shader {
public:
    GLuint ID = 0;

    Shader() = default;

    Shader(const char* vertexName, const char* fragmentName);

    Shader(const char* computeName, bool hasExtra = false, const char* extraCode = "");

    void use() const;

    // utility uniform functions
    void setBool(const char* name, int len, bool value) const;
    // ------------------------------------------------------------------------
    void setInt(const char* name, int len, int value) const;
    // ------------------------------------------------------------------------
    void setFloat(const char* name, int len, float value) const;
    // ------------------------------------------------------------------------
    void setVec2(const char* name, int len, const vec2& value) const;
    void setVec2(const char* name, int len, float x, float y) const;
    // ------------------------------------------------------------------------
    void setVec3(const char* name, int len, const vec3& value) const;
    void setVec3(const char* name, int len, float x, float y, float z) const;

private:
    mutable std::vector<int> uniformCache;
    void cacheUniforms();

    GLint getUniformLocation(const char* uniformName, int len) const;
};