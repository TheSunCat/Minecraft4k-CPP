#pragma once
#include "Constants.h"

namespace World
{
    extern uint8_t* world;

    void setBlock(int x, int y, int z, uint8_t block);

    uint8_t getBlock(int x, int y, int z);

    uint8_t getBlock(const vec3& pos);

    bool isWithinWorld(const vec3& pos);

    void fillBox(uint8_t blockId, const vec3& pos0,
        const vec3& pos1, bool replace);

    vec3 raycast(vec3 origin, vec3 dir, float maxDist, int& hitAxis);

    void generateWorld(); // randomize seed
    void generateWorld(uint64_t seed);
}