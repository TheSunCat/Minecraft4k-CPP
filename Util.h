#pragma once

#include <cstdint>

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

    int next(const int bits);

public:
    Random(const long seed);

    Random();

    float nextFloat();

    int nextInt();

    int nextInt(const int bound);

    void setSeed(const uint64_t newSeed);
};

// It's just Perlin from Processing
namespace Perlin
{
    float noise(float x, float y);
}

float clamp(float val, const float min, const float max);