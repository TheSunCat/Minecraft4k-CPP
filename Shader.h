#pragma once

#include <string>
#include <unordered_map>

#include <glad/glad.h>
#include <glm/glm.hpp>

enum class HasExtra {Yes, No};

class Shader {
public:
    GLuint ID = 0;

    Shader() = default;

    Shader(std::string vertexName, std::string fragmentName);

    Shader(std::string computeName, HasExtra hasExtra, const char* extraCode = "");

    Shader(HasExtra, std::string source);

    void use() const;

    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    // ------------------------------------------------------------------------
    void setInt(const std::string& name, int value) const;
    // ------------------------------------------------------------------------
    void setFloat(const std::string& name, float value) const;
    // ------------------------------------------------------------------------
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec2(const std::string& name, float x, float y) const;
    // ------------------------------------------------------------------------
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    // ------------------------------------------------------------------------
    void setVec4(const std::string& name, const glm::vec4& value) const;
    void setVec4(const std::string& name, float x, float y, float z, float w) const;
    // ------------------------------------------------------------------------
    void setMat2(const std::string& name, const glm::mat2& mat) const;
    // ------------------------------------------------------------------------
    void setMat3(const std::string& name, const glm::mat3& mat) const;
    // ------------------------------------------------------------------------
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    GLint getUniformLocation(const char* uniformName) const;

    mutable std::unordered_map<std::string, GLint> uniformCache;
};