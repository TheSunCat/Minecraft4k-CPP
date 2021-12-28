#pragma once

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <glad/glad.h>

#include "Vector.h"

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

    vec2 nextVec2(float magnitude);


    uint32_t nextInt();

    vec2 nextIVec2(int magnitude);

    uint32_t nextInt(uint32_t bound);

    uint64_t nextLong();


    void setSeed(uint64_t newSeed);
};

// It's just Perlin from Processing
namespace Perlin
{
    float noise(vec2 pos);
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

vec3 rotToVec3(float yaw, float pitch);

float radians(float deg);
float degrees(float rad);

// why do I have to define these??
/*
float absf(float v);
float maxf(float a, float b);*/
float roundFloat(float v);
bool sign(float v);
float fract(float v);

vec3 max(const vec3& a, const vec3& b);

unsigned int murmurHash2(const char* str, int len);

/* reverse:  reverse string s in place */
void reverse(char s[]);

/* itoa:  convert n to characters in s */
void itoa(int n, char s[]);

#define PASS_STR(a) a, strlen(a)