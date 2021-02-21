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

constexpr float maxTerrainHeight = WORLD_HEIGHT / 2.0f;

#ifdef CLASSIC // classic worldgen
void World::generateWorld(long long seed)
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
constexpr float halfWorldSize = WORLD_SIZE / 2.0f;
constexpr int stoneDepth = 5;

void World::generateWorld(long long seed)
{
    Random rand = Random(seed);

    for (int x = WORLD_SIZE; x >= 0; x--) {
        for (int z = 0; z < WORLD_SIZE; z++) {
            const int terrainHeight = round(maxTerrainHeight + Perlin::noise(x / halfWorldSize, z / halfWorldSize) * 10.0f);

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
                const int treeX = x + (rand.nextInt(4) - 2);
                const int treeZ = z + (rand.nextInt(4) - 2);

                const int terrainHeight = round(maxTerrainHeight + Perlin::noise(treeX / halfWorldSize, treeZ / halfWorldSize) * 10.0f) - 1;

                const int treeHeight = 4 + rand.nextInt(2); // min 4 max 5

                for (int y = terrainHeight; y >= terrainHeight - treeHeight; y--)
                {
                    setBlock(treeX, y, treeZ, BLOCK_WOOD);
                }

                // foliage
                fillBox(BLOCK_LEAVES,
                    glm::vec3(treeX - 2, terrainHeight - treeHeight + 1, treeZ - 2),
                    glm::vec3(treeX + 3, terrainHeight - treeHeight + 3, treeZ + 3), false);

                // crown
                fillBox(BLOCK_LEAVES,
                    glm::vec3(treeX - 1, terrainHeight - treeHeight - 1, treeZ - 1),
                    glm::vec3(treeX + 2, terrainHeight - treeHeight + 1, treeZ + 2), false);


                int foliageXList[] = { treeX - 2, treeX - 2, treeX + 2, treeX + 2 };
                int foliageZList[] = { treeZ - 2, treeZ + 2, treeZ + 2, treeZ - 2 };

                int crownXList[] = { treeX - 1, treeX - 1, treeX + 1, treeX + 1 };
                int crownZList[] = { treeZ - 1, treeZ + 1, treeZ + 1, treeZ - 1 };

                for (int i = 0; i < 4; i++)
                {
                    const int foliageX = foliageXList[i];
                    const int foliageZ = foliageZList[i];

                    const int foliageCut = rand.nextInt(10);

                    switch (foliageCut) {
                    case 0: // cut out top
                        setBlock(foliageX, terrainHeight - treeHeight + 1, foliageZ, BLOCK_AIR);
                        break;
                    case 1: // cut out bottom
                        setBlock(foliageX, terrainHeight - treeHeight + 2, foliageZ, BLOCK_AIR);
                        break;
                    case 2: // cut out both
                        setBlock(foliageX, terrainHeight - treeHeight + 1, foliageZ, BLOCK_AIR);
                        setBlock(foliageX, terrainHeight - treeHeight + 2, foliageZ, BLOCK_AIR);
                        break;
                    default: // do nothing
                        break;
                    }


                    const int crownX = crownXList[i];
                    const int crownZ = crownZList[i];

                    const int crownCut = rand.nextInt(10);

                    switch (crownCut) {
                    case 0: // cut out both
                        setBlock(crownX, terrainHeight - treeHeight - 1, crownZ, BLOCK_AIR);
                        setBlock(crownX, terrainHeight - treeHeight, crownZ, BLOCK_AIR);
                        break;
                    default: // do nothing
                        setBlock(crownX, terrainHeight - treeHeight - 1, crownZ, BLOCK_AIR);
                        break;
                    }
                }
            }
        }
    }
}
#endif