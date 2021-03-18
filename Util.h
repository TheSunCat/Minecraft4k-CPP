#pragma once

#include <cstdint>
#include <iostream>
#include <glad/glad.h>
#include <glm/glm.hpp>

constexpr float PI = 3.14159265359f;

float currentTime();

// It's just the Java Random class
class Random
{
    uint64_t seed = 0;

    static uint64_t seedUniquifier;

    constexpr static uint64_t multiplier = 0x5DEECE66D;
    constexpr static uint64_t addend = 0xBL;
    constexpr static uint64_t mask = (uint64_t(1) << 48) - 1;

    static uint64_t initialScramble(const uint64_t seed);

    static uint64_t uniqueSeed();

    int next(int bits);

public:
    Random(uint64_t seed);

    Random();

    float nextFloat();

    glm::vec2 nextVec2(float magnitude);


    uint32_t nextInt();

    glm::ivec2 nextIVec2(int magnitude);

    uint32_t nextInt(uint32_t bound);

    uint64_t nextLong();


    void setSeed(uint64_t newSeed);
};

// It's just Perlin from Processing
namespace Perlin
{
    float noise(glm::vec2 pos);
    float noise(float x, float y);
}

float clamp(float val, float min, float max);

bool glError();

void GLAPIENTRY error_callback(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam);

std::ostream& operator<<(std::ostream& os, const glm::vec3& vec3);

glm::vec3 rotToVec3(float yaw, float pitch);