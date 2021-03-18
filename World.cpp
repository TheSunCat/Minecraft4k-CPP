#include "World.h"
#include "Util.h"

uint8_t* World::world = new uint8_t[WORLD_SIZE * WORLD_HEIGHT * WORLD_SIZE];

void World::setBlock(const int x, const int y, const int z, const uint8_t block)
{
    world[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_HEIGHT] = block;
}

uint8_t World::getBlock(const int x, const int y, const int z)
{
    return world[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_HEIGHT];
}

uint8_t World::getBlock(const glm::vec3& pos)
{
    return World::getBlock(pos.x, pos.y, pos.z);
}

bool World::isWithinWorld(const glm::vec3& pos)
{
    return pos.x >= 0.0f && pos.y >= 0.0f && pos.z >= 0.0f &&
        pos.x < WORLD_SIZE && pos.y < WORLD_HEIGHT && pos.z < WORLD_SIZE;
}

void World::fillBox(const uint8_t blockId, const glm::vec3& pos0,
    const glm::vec3& pos1, const bool replace)
{
    for (int x = pos0.x; x < pos1.x; x++)
    {
        for (int y = pos0.y; y < pos1.y; y++)
        {
            for (int z = pos0.z; z < pos1.z; z++)
            {
                if (!replace) {
                    if (getBlock(x, y, z) != BLOCK_AIR)
                        continue;
                }

                setBlock(x, y, z, blockId);
            }
        }
    }
}

glm::vec3 World::raycast(glm::vec3 origin, glm::vec3 dir, float maxDist, int& hitAxis)
{
    //glm::ivec3 iOrigin = glm::ivec3(origin); // Integer version of start vec

    // Determine the chunk-relative position of the ray using a bit-mask
    int i = origin.x, j = origin.y, k = origin.z;

    // The amount to increase i, j and k in each axis (either 1 or -1)
    glm::ivec3 ijkStep = glm::ivec3(glm::sign(dir));

    // This variable is used to track the current progress throughout the ray march
    glm::vec3 vInverted = glm::abs(1.0F / dir);

    // The distance to the closest voxel boundary in units of rayTravelDist
    glm::vec3 dist = glm::fract(origin) * glm::vec3(ijkStep);
    dist += glm::max(ijkStep, glm::ivec3(0));
    dist *= vInverted;

    int axis = 0;

    float rayTravelDist = 0;

    while (rayTravelDist <= maxDist)
    {
        // Exit check
        if(!World::isWithinWorld(glm::ivec3(i, j, k)))
            break;

        int blockHit = getBlock(glm::ivec3(i, j, k));

        if (blockHit != BLOCK_AIR)
        {
            glm::vec3 hitPos = origin + dir * rayTravelDist;

            hitAxis = axis;
            if(dir[hitAxis] > 0.0F)
                hitAxis += 3;

            return hitPos;// origin + dir * (rayTravelDist - 0.01f);
        }

        // Determine the closest voxel boundary
        if (dist.y < dist.x)
        {
            if (dist.y < dist.z)
            {
                // Advance to the closest voxel boundary in the Y direction

                // Increment the chunk-relative position and the block access position
                j += ijkStep.y;

                // Update our progress in the ray 
                rayTravelDist = dist.y;

                // Set the new distance to the next voxel Y boundary
                dist.y += vInverted.y;

                // For collision purposes we also store the last axis that the ray collided with
                // This allows us to reflect particle velocity on the correct axis
                axis = 1;//AXIS_Y;
            }
            else
            {
                k += ijkStep.z;

                rayTravelDist = dist.z;
                dist.z += vInverted.z;
                axis = 2;//AXIS_Z;
            }
        }
        else if (dist.x < dist.z)
        {
            i += ijkStep.x;

            rayTravelDist = dist.x;
            dist.x += vInverted.x;
            axis = 0;//AXIS_X;
        }
        else
        {
            k += ijkStep.z;

            rayTravelDist = dist.z;
            dist.z += vInverted.z;
            axis = 2;//AXIS_Z;
        }
    }

    return glm::vec3(-1); // no hit
}

void World::generateWorld()
{
    Random rand;
    uint64_t seed = rand.nextLong();

    generateWorld(seed);
}

constexpr float maxTerrainHeight = WORLD_HEIGHT / 2.0f;

#ifdef CLASSIC // classic worldgen
void World::generateWorld(uint64_t seed)
{
    Random rand = Random(seed);
    for (int x = WORLD_SIZE; x >= 0; x--) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int z = 0; z < WORLD_SIZE; z++) {
                uint8_t block;

                if (y > maxTerrainHeight + rand.nextInt(8))
                    block = rand.nextInt(8) + 1;
                else
                    block = BLOCK_AIR;

                if (x == WORLD_SIZE)
                    continue;

                setBlock(x, y, z, block);
            }
        }
    }
}
#else // new worldgen
constexpr int stoneDepth = 5;

void World::generateWorld(uint64_t seed)
{
    Random rand = Random(seed);

    for (int x = WORLD_SIZE; x >= 0; x--) {
        for (int z = 0; z < WORLD_SIZE; z++) {
            const int terrainHeight = round(maxTerrainHeight + Perlin::noise(x / 32.f, z / 32.f) * 10.0f);

            for (int y = 0; y < WORLD_HEIGHT; y++) {
                uint8_t block;

                if (y > terrainHeight + stoneDepth)
                    block = BLOCK_STONE;
                else if (y > terrainHeight)
                    block = BLOCK_DEFAULT_DIRT;
                else if (y == terrainHeight)
                    block = BLOCK_GRASS;
                else
                    block = BLOCK_AIR;

                setBlock(x, y, z, block);
            }
        }
    }

    // populate trees
    for (int x = 4; x < WORLD_SIZE - 4; x += 8) {
        for (int z = 4; z < WORLD_SIZE - 4; z += 8) {
            if (rand.nextInt(4) == 0) // spawn tree
            {
                const glm::vec2 treePos = rand.nextIVec2(2) + glm::ivec2(x, z);

                const int terrainHeight = int(round(maxTerrainHeight + Perlin::noise(treePos / 32.f) * 10.0f)) - 1;
                const int trunkHeight = 4 + rand.nextInt(2); // min 4 max 5

                // fill trunk
                for (int y = terrainHeight; y >= terrainHeight - trunkHeight; y--)
                {
                    setBlock(treePos.s, y, treePos.t, BLOCK_WOOD);
                }

                // fill base foliage
                fillBox(BLOCK_LEAVES,
                    glm::vec3(treePos.s - 2, terrainHeight - trunkHeight + 1, treePos.t - 2),
                    glm::vec3(treePos.s + 3, terrainHeight - trunkHeight + 3, treePos.t + 3), false);

                // fill crown
                fillBox(BLOCK_LEAVES,
                    glm::vec3(treePos.s - 1, terrainHeight - trunkHeight - 1, treePos.t - 1),
                    glm::vec3(treePos.s + 2, terrainHeight - trunkHeight + 1, treePos.t + 2), false);

                // cut out corners randomly
                for (int i = 0; i < 4; i++)
                {
                    // binary counting, so we cover all values
                    int bit0 = (i >> 0 & 0b01) * 2 - 1;
                    int bit1 = (i >> 1 & 0b01) * 2 - 1;


                    // base foliage
                    const glm::ivec2 foliagePos = glm::ivec2(treePos.s + (2 * bit0), treePos.t + (2 * bit1));


                    int cornerStyle = rand.nextInt(7);

                    if ((cornerStyle == 0) || (cornerStyle == 2)) // cut out top
                       setBlock(foliagePos.s, terrainHeight - trunkHeight + 1, foliagePos.t, BLOCK_AIR);

                    if ((cornerStyle == 1) || (cornerStyle == 2)) // cut out bottom
                        setBlock(foliagePos.s, terrainHeight - trunkHeight + 2, foliagePos.t, BLOCK_AIR);


                    // crown
                    const glm::ivec2 crownPos = glm::ivec2(treePos.s + bit0, treePos.t + bit1);

                    cornerStyle = rand.nextInt(5);

                    if (cornerStyle == 0) // cut out bottom 1/10 times
                        setBlock(crownPos.s, terrainHeight - trunkHeight, crownPos.t, BLOCK_AIR);

                    // always cut crown top
                    setBlock(crownPos.s, terrainHeight - trunkHeight - 1, crownPos.t, BLOCK_AIR);
                }
            }
        }
    }
}
#endif