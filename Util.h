#pragma once

#include <glad.h>

#include "Constants.h"
#include "Vector.h"

float currentTime();

constexpr float PI = 3.14159265359f;

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

float radians(float deg);
float degrees(float rad);

int roundFloat(float v);
//float abs(float v);
bool sign(float v);
float fract(float v);
float pow(float v, int p);
float mod(float v, float d);

vec3 max(const vec3& a, const vec3& b);

unsigned int murmurHash2(const char* str, int len);

/*
unsigned long strlen(const char *str);
char *strcpy(char *strDest, const char *strSrc);
char *strcat(char *dest, const char *src);
*/
void memcpy(void *dest, void *src, long unsigned int n);

/* reverse:  reverse string s in place */
void reverse(char s[]);

/* itoa:  convert n to characters in s */
void itoa(int n, char s[]);

#define PASS_STR(a) a, strlen(a)


#ifdef DEBUG
#include <cstdio>
#define prints(a) fputs(a, stdout)
#else
#define prints(a)
#endif
void crash();
