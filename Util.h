#pragma once

#include <cstdint>
#include <iostream>
#include <glad/glad.h>
#include <glm/vec3.hpp>

constexpr float PI = 3.14159265359f;

long long currentTime();

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

    int nextInt();

    int nextInt(int bound);

    void setSeed(uint64_t newSeed);
};

// It's just Perlin from Processing
namespace Perlin
{
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