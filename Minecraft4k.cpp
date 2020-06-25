#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Util.h"


struct Controller
{
    float forward;
    float right;

    bool jump;

    glm::vec2 lastMousePos;

    bool firstMouse = true;
};

//#define CLASSIC

Controller controller{};

bool needsResUpdate = true;

int SCR_DETAIL = 1;

int SCR_RES_X = 107 * pow(2, SCR_DETAIL);
int SCR_RES_Y = 60 * pow(2, SCR_DETAIL);

Shader renderShader;
GLuint computeProgram;
GLuint buffer;
GLuint vao;

GLuint textureAtlasTex;
GLuint worldTexture;
GLuint screenTexture;

constexpr int WINDOW_WIDTH = 856;
constexpr int WINDOW_HEIGHT = 480;

#ifdef CLASSIC
constexpr float RENDER_DIST = 20.0f;
#else
constexpr float RENDER_DIST = 80.0f;
#endif

constexpr float PLAYER_REACH = 5.0f;

constexpr int TEXTURE_RES = 16;

constexpr int WORLD_SIZE = 64;
constexpr int WORLD_HEIGHT = 64;

constexpr unsigned char BLOCK_AIR = 0;
constexpr unsigned char BLOCK_GRASS = 1;
constexpr unsigned char BLOCK_DEFAULT_DIRT = 2;
constexpr unsigned char BLOCK_STONE = 4;
constexpr unsigned char BLOCK_BRICKS = 5;
constexpr unsigned char BLOCK_WOOD = 7;
constexpr unsigned char BLOCK_LEAVES = 8;

// COLORS
constexpr glm::vec3 FOG_COLOR = glm::vec3(1);

// S = Sun, A = Amb, Y = skY
constexpr glm::vec3 SC_DAY = glm::vec3(1);
constexpr glm::vec3 AC_DAY = glm::vec3(0.5f, 0.5f, 0.5f);
constexpr glm::vec3 YC_DAY = glm::vec3(0x51BAF7);

constexpr glm::vec3 SC_TWILIGHT = glm::vec3(1, 0.5f, 0.01f);
constexpr glm::vec3 AC_TWILIGHT = glm::vec3(0.6f, 0.5f, 0.5f);
constexpr glm::vec3 YC_TWILIGHT = glm::vec3(0.27f, 0.24f, 0.33f);

constexpr glm::vec3 SC_NIGHT = glm::vec3(0.3f, 0.3f, 0.5f);
constexpr glm::vec3 AC_NIGHT = glm::vec3(0.3f, 0.3f, 0.5f);
constexpr glm::vec3 YC_NIGHT = glm::vec3(0.004f, 0.004f, 0.008f);

long deltaTime = 0;

glm::vec3 playerPos = glm::vec3(WORLD_SIZE + WORLD_SIZE / 2.0f + 0.5f, 
                                WORLD_HEIGHT + 1, 
                                WORLD_SIZE + WORLD_SIZE / 2.0f + 0.5f);
glm::vec3 playerVelocity;

glm::vec3 hoveredBlockPos;
glm::vec3 placeBlockPos;

glm::vec3 newHoverBlockPos;

glm::vec3 lightDirection = glm::vec3(0.866025404f, -0.866025404f, 0.866025404f);

float cameraYaw = 0.0f;
float cameraPitch = 0.0f;
float FOV = 90.0f;

float sinYaw, sinPitch;
float cosYaw, cosPitch;

glm::vec3 sunColor;
glm::vec3 ambColor;
glm::vec3 skyColor;

uint8_t world[WORLD_SIZE * WORLD_HEIGHT * WORLD_SIZE];

unsigned char hotbar[] { BLOCK_GRASS, BLOCK_DEFAULT_DIRT, BLOCK_STONE, BLOCK_BRICKS, BLOCK_WOOD, BLOCK_LEAVES };
int heldBlockIndex = 0;

static void setBlock(const int x, const int y, const int z, const int block)
{
    world[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_HEIGHT] = block;
}

static int getBlock(const int x, const int y, const int z)
{
    return world[x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_HEIGHT];
}

static bool isWithinWorld(const glm::vec3& pos)
{
    return pos.x >= 0.0f && pos.y >= 0.0f && pos.z >= 0.0f &&
        pos.x < WORLD_SIZE&& pos.y < WORLD_HEIGHT&& pos.z < WORLD_SIZE;
}

static void fillBox(const unsigned char blockId, const glm::vec3& pos0,
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

static glm::vec3 lerp(const glm::vec3& start, const glm::vec3& end, const float t)
{
    return glm::vec3(start + (start - end) * t);
}

void updateScreenResolution()
{
    SCR_RES_X = 107 * pow(2, SCR_DETAIL);
    SCR_RES_Y = 60 * pow(2, SCR_DETAIL);

    // auto generated code - do not delete
    std::string title = "Minecraft4k";

    switch (SCR_RES_X) {
    case 6:
        title += " on battery-saving mode";
        break;
    case 13:
        title += " on a potato";
        break;
    case 26:
        title += " on an undocked switch";
        break;
    case 53:
        title += " on a TI-84";
        break;
    case 107:
        title += " on an Atari 2600";
        break;
    case 428:
        title += " at SD";
        break;
    case 856:
        title += " at HD";
        break;
    case 1712:
        title += " at Full HD";
        break;
    case 3424:
        title += " at 4K";
        break;
    case 6848:
        title += " on a NASA supercomputer";
        break;
    }

    //TODO frame.setTitle(title);

    needsResUpdate = false;
}

void init()
{
    Random rand = Random(18295169L);

    // generate world

    float maxTerrainHeight = WORLD_HEIGHT / 2.0f;
#ifdef CLASSIC
    for (int x = WORLD_SIZE; x >= 0; x--) {
        for (int y = 0; y < WORLD_HEIGHT; y++) {
            for (int z = 0; z < WORLD_SIZE; z++) {
                unsigned char block;

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
#else
    float halfWorldSize = WORLD_SIZE / 2.0f;

    constexpr int stoneDepth = 5;

    for (int x = 0; x < WORLD_SIZE; x++) {
        for (int z = 0; z < WORLD_SIZE; z++) {
            int terrainHeight = round(maxTerrainHeight + Perlin::noise(x / halfWorldSize, z / halfWorldSize) * 10.0f);

            for (int y = terrainHeight; y < WORLD_HEIGHT; y++)
            {
                unsigned char block;

                if (y > terrainHeight + stoneDepth)
                    block = BLOCK_STONE;
                else if (y > terrainHeight)
                    block = 2; // dirt
                else // (y == terrainHeight)
                    block = BLOCK_GRASS;

                setBlock(x, y, z, block);
            }
        }
    }

    // populate trees
    for (int x = 4; x < WORLD_SIZE - 4; x += 8) {
        for (int z = 4; z < WORLD_SIZE - 4; z += 8) {
            if (rand.nextInt(4) == 0) // spawn tree
            {
                int treeX = x + (rand.nextInt(4) - 2);
                int treeZ = z + (rand.nextInt(4) - 2);

                int terrainHeight = round(maxTerrainHeight + Perlin::noise(treeX / halfWorldSize, treeZ / halfWorldSize) * 10.0f) - 1;

                int treeHeight = 4 + rand.nextInt(2); // min 4 max 5

                for (int y = terrainHeight; y >= terrainHeight - treeHeight; y--)
                {
                    unsigned char block = BLOCK_WOOD;

                    setBlock(treeX, y, treeZ, block);
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
                    int foliageX = foliageXList[i];
                    int foliageZ = foliageZList[i];

                    int foliageCut = rand.nextInt(10);

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


                    int crownX = crownXList[i];
                    int crownZ = crownZList[i];

                    int crownCut = rand.nextInt(10);

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
#endif

    // set random seed to generate textures
    rand.setSeed(151910774187927L);


    int* textureAtlas = new int[TEXTURE_RES * TEXTURE_RES * 3 * 16];

    // procedurally generates the 16x3 textureAtlas
    // gsd = grayscale detail
    for (int blockType = 1; blockType < 16; blockType++) {
        int gsd_tempA = 0xFF - rand.nextInt(0x60);

        for (int y = 0; y < TEXTURE_RES * 3; y++) {
            for (int x = 0; x < TEXTURE_RES; x++) {
                // gets executed per pixel/texel

                int gsd_constexpr;
                int tint;

#ifdef CLASSIC
                if (blockType != BLOCK_STONE || rand.nextInt(3) == 0) // if the block type is stone, update the noise value less often to get a stretched out look
                    gsd_tempA = 0xFF - rand.nextInt(0x60);

                tint = 0x966C4A; // brown (dirt)
                switch (blockType)
                {
                case BLOCK_STONE:
                {
                    tint = 0x7F7F7F; // grey
                    break;
                }
                case BLOCK_GRASS:
                {
                    if (y < (x * x * 3 + x * 81 >> 2 & 0x3) + (TEXTURE_RES * 1.125f)) // grass + grass edge
                        tint = 0x6AAA40; // green
                    else if (y < (x * x * 3 + x * 81 >> 2 & 0x3) + (TEXTURE_RES * 1.1875f)) // grass edge shadow
                        gsd_tempA = gsd_tempA * 2 / 3;
                    break;
                }
                case BLOCK_WOOD:
                {
                    tint = 0x675231; // brown (bark)
                    if (!(y >= TEXTURE_RES && y < TEXTURE_RES * 2) && // second row = stripes
                        x > 0 && x < TEXTURE_RES - 1 &&
                        ((y > 0 && y < TEXTURE_RES - 1) || (y > TEXTURE_RES * 2 && y < TEXTURE_RES * 3 - 1))) { // wood side area
                        tint = 0xBC9862; // light brown

                        // the following code repurposes 2 gsd variables making it a bit hard to read
                        // but in short it gets the absolute distance from the tile's center in x and y direction 
                        // finds the max of it
                        // uses that to make the gray scale detail darker if the current pixel is part of an annual ring
                        // and adds some noise as a finishing touch
                        int woodCenter = TEXTURE_RES / 2 - 1;

                        int dx = x - woodCenter;
                        int dy = (y % TEXTURE_RES) - woodCenter;

                        if (dx < 0)
                            dx = 1 - dx;

                        if (dy < 0)
                            dy = 1 - dy;

                        if (dy > dx)
                            dx = dy;

                        gsd_tempA = 196 - rand.nextInt(32) + dx % 3 * 32;
                    }
                    else if (rand.nextInt(2) == 0) {
                        // make the gsd 50% brighter on random pixels of the bark
                        // and 50% darker if x happens to be odd
                        gsd_tempA = gsd_tempA * (150 - (x & 1) * 100) / 100;
                    }
                    break;
                }
                case BLOCK_BRICKS:
                {
                    tint = 0xB53A15; // red
                    if ((x + y / 4 * 4) % 8 == 0 || y % 4 == 0) // gap between bricks
                        tint = 0xBCAFA5; // reddish light grey
                    break;
                }
                }

                gsd_constexpr = gsd_tempA;
                if (y >= TEXTURE_RES * 2) // bottom side of the block
                    gsd_constexpr /= 2; // make it darker, baked "shading"

                if (blockType == BLOCK_LEAVES) {
                    tint = 0x50D937; // green
                    if (rand.nextInt(2) == 0) {
                        tint = 0;
                        gsd_constexpr = 0xFF;
                    }
            }
#else
                float pNoise = Perlin::noise(x, y);

                tint = 0x966C4A; // brown (dirt)

                gsd_tempA = (1 - pNoise * 0.5f) * 255;
                switch (blockType) {
                case BLOCK_STONE:
                {
                    tint = 0x7F7F7F; // grey
                    gsd_tempA = double(0.75 + round(abs(Perlin::noise(x * 0.5f, y * 2))) * 0.125) * 255;
                    break;
                }
                case BLOCK_GRASS:
                {
                    if (y < (((x * x * 3 + x * 81) / 2) % 4) + 18) // grass + grass edge
                        tint = 0x7AFF40; //green
                    else if (y < (((x * x * 3 + x * 81) / 2) % 4) + 19)
                        gsd_tempA = gsd_tempA * 1 / 3;
                    break;
                }
                case BLOCK_WOOD:
                {
                    tint = 0x776644; // brown (bark)

                    int woodCenter = TEXTURE_RES / 2 - 1;
                    int dx = x - woodCenter;
                    int dy = (y % TEXTURE_RES) - woodCenter;

                    if (dx < 0)
                        dx = 1 - dx;

                    if (dy < 0)
                        dy = 1 - dy;

                    if (dy > dx)
                        dx = dy;

                    double distFromCenter = (sqrt(dx * dx + dy * dy) * .25f + std::max(dx, dy) * .75f);

                    if (y < 16 || y > 32) { // top/bottom
                        if (distFromCenter < float(TEXTURE_RES) / 2.0f)
                        {
                            tint = 0xCCAA77; // light brown

                            gsd_tempA = 196 - rand.nextInt(32) + dx % 3 * 32;
                        }
                        else if (dx > dy) {
                            gsd_tempA = Perlin::noise(y, x * .25f) * 255 * (180 - sin(x * PI) * 50) / 100;
                        }
                        else {
                            gsd_tempA = Perlin::noise(x, y * .25f) * 255 * (180 - sin(x * PI) * 50) / 100;
                        }
                    }
                    else { // side texture
                        gsd_tempA = Perlin::noise(x, y * .25f) * 255 * (180 - sin(x * PI) * 50) / 100;
                    }
                    break;
                }
                case BLOCK_BRICKS:
                {
                    tint = 0x444444; // red

                    float brickDX = abs(x % 8 - 4);
                    float brickDY = abs((y % 4) - 2) * 2;

                    if ((y / 4) % 2 == 1)
                        brickDX = abs((x + 4) % 8 - 4);

                    float d = sqrt(brickDX * brickDX + brickDY * brickDY) * .5f
                        + std::max(brickDX, brickDY) * .5f;

                    if (d > 4) // gap between bricks
                        tint = 0xAAAAAA; // light grey
                    break;
                }
                }

                gsd_constexpr = gsd_tempA;

                if (blockType == BLOCK_LEAVES)
                {
                    tint = 0;

                    float dx = abs(x % 4 - 2) * 2;
                    float dy = (y % 8) - 4;

                    if ((y / 8) % 2 == 1)
                        dx = abs((x + 2) % 4 - 2) * 2;

                    dx += pNoise;

                    float d = dx + abs(dy);

                    if (dy < 0)
                        d = sqrt(dx * dx + dy * dy);

                    if (d < 3.5f)
                        tint = 0xFFCCDD;
                    else if (d < 4)
                        tint = 0xCCAABB;
                }
#endif

                // multiply tint by the grayscale detail
                const int col = ((tint & 0xFFFFFF) == 0 ? 0 : 0xFF) << 24 |
			                    (tint >> 16 & 0xFF) * gsd_constexpr / 0xFF << 16 |
			                    (tint >>  8 & 0xFF) * gsd_constexpr / 0xFF << 8 |
			                    (tint & 0xFF      ) * gsd_constexpr / 0xFF << 0;

                // write pixel to the texture atlas
                textureAtlas[x + y * TEXTURE_RES + blockType * (TEXTURE_RES * TEXTURE_RES) * 3] = col;
        }
    }
    }
	
    glGenTextures(1, &textureAtlasTex);
    glBindTexture(GL_TEXTURE_2D, textureAtlasTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, TEXTURE_RES * 3, TEXTURE_RES * 16);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXTURE_RES * 3, TEXTURE_RES * 16, GL_BGRA, GL_UNSIGNED_BYTE, textureAtlas);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] textureAtlas;
	
	glGenTextures(1, &worldTexture);
	glBindTexture(GL_TEXTURE_3D, worldTexture);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, WORLD_SIZE, WORLD_HEIGHT, WORLD_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, world);
	glBindTexture(GL_TEXTURE_3D, 0);
}

void collidePlayer()
{
    // check for movement on each axis individually?
    for (int axisIndex = 0; axisIndex < 3; axisIndex++) {
        if (false) {
        OUTER:axisIndex++;
            if (axisIndex >= 3)
                return;
        }
        const glm::vec3 newPlayerPos = glm::vec3(playerPos.x + playerVelocity.x * ((axisIndex + 1) % 3 / 2),
									             playerPos.y + playerVelocity.y * ((axisIndex + 0) % 3 / 2),
											     playerPos.z + playerVelocity.z * ((axisIndex + 2) % 3 / 2));

        for (int colliderIndex = 0; colliderIndex < 12; colliderIndex++) {
            // magic
            const glm::ivec3 colliderBlockPos = glm::ivec3((newPlayerPos.x + (colliderIndex & 1) * 0.6F - 0.3F) - WORLD_SIZE,
											               (newPlayerPos.y + ((colliderIndex >> 2) - 1) * 0.8F + 0.65F) - WORLD_HEIGHT,
											               (newPlayerPos.z + (colliderIndex >> 1 & 1) * 0.6F - 0.3F) - WORLD_SIZE);

            if (colliderBlockPos.y < 0)
                continue;

            // check collision with world bounds and world blocks
            if (!isWithinWorld(colliderBlockPos)
                || getBlock(colliderBlockPos.x, colliderBlockPos.y, colliderBlockPos.z) != BLOCK_AIR) {

                if (axisIndex != 2) // not checking for vertical movement
                    goto OUTER; // movement is invalid

                // if we're falling, colliding, and we press space
                if (controller.jump && playerVelocity.y > 0.0F) {
                    playerVelocity.y = -0.1F; // jump
                    return;
                }

                // stop vertical movement
                playerVelocity.y = 0.0F;
                return;
            }
        }

        playerPos = newPlayerPos;
    }
}

void run(GLFWwindow* window) {
    long startTime = currentTime();

    while (!glfwWindowShouldClose(window)) {
	    const long time = currentTime();

        if (needsResUpdate) {
            updateScreenResolution();
        }
        

        sinYaw = sin(cameraYaw);
        cosYaw = cos(cameraYaw);
        sinPitch = sin(cameraPitch);
        cosPitch = cos(cameraPitch);

        lightDirection.y = sin(time / 10000000.0);

        lightDirection.x = 0; //lightDirection.y * 0.5f;
        lightDirection.z = cos(time / 10000000.0);


        if (lightDirection.y < 0.0f)
        {
            sunColor = lerp(SC_TWILIGHT, SC_DAY, -lightDirection.y);
            ambColor = lerp(AC_TWILIGHT, AC_DAY, -lightDirection.y);
            skyColor = lerp(YC_TWILIGHT, YC_DAY, -lightDirection.y);
        }
        else {
            sunColor = lerp(SC_TWILIGHT, SC_NIGHT, lightDirection.y);
            ambColor = lerp(AC_TWILIGHT, AC_NIGHT, lightDirection.y);
            skyColor = lerp(YC_TWILIGHT, YC_NIGHT, lightDirection.y);
        }


	    const float inputX = controller.right * 0.02F;
	    const float inputZ = controller.forward * 0.02F;
        
        playerVelocity.x *= 0.5F;
        playerVelocity.y *= 0.99F;
        playerVelocity.z *= 0.5F;
        
        playerVelocity.x += sinYaw * inputZ + cosYaw * inputX;
        playerVelocity.z += cosYaw * inputZ - sinYaw * inputX;
        playerVelocity.y += 0.003F; // gravity

		collidePlayer();
    	
    	
        for (int colliderIndex = 0; colliderIndex < 12; colliderIndex++) {
            int magicX = int(playerPos.x + ( colliderIndex       & 1) * 0.6F - 0.3F) - WORLD_SIZE;
            int magicY = int(playerPos.y + ((colliderIndex >> 2) - 1) * 0.8F + 0.65F) - WORLD_HEIGHT;
            int magicZ = int(playerPos.z + ( colliderIndex >> 1  & 1) * 0.6F - 0.3F) - WORLD_SIZE;

            // set block to air if inside player
            if (isWithinWorld(glm::vec3(magicX, magicY, magicZ)))
                setBlock(magicX, magicY, magicZ, BLOCK_AIR);
        }

    	// Compute the raytracing!
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    	
        glUseProgram(computeProgram);

        glUniform2f(glGetUniformLocation(computeProgram, "uSize"), SCR_RES_X, SCR_RES_Y);
        glUniform2f(glGetUniformLocation(computeProgram, "screenSize"), SCR_RES_X, SCR_RES_Y);
    	
        //glBindImageTexture(2, textureAtlasTex, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA8UI);
        glBindTexture(GL_TEXTURE_2D, textureAtlasTex);
        glUniform1i(glGetUniformLocation(computeProgram, "textureAtlas"), 0);

        glBindTexture(GL_TEXTURE_3D, worldTexture);
        glBindImageTexture(1, worldTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R8UI);
    	glError();
    	
        glUniform1f(glGetUniformLocation(computeProgram, "camera.yaw"), cameraYaw);
        glUniform1f(glGetUniformLocation(computeProgram, "camera.pitch"), cameraPitch);
        glUniform1f(glGetUniformLocation(computeProgram, "camera.FOV"), 90);

        glUniform3fv(glGetUniformLocation(computeProgram, "playerPos"), 1, &playerPos[0]);
        glUniform3fv(glGetUniformLocation(computeProgram, "lightDirection"), 1, &lightDirection[0]);

        glError();
    	
        glDispatchCompute(SCR_RES_X, SCR_RES_Y, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glError();
    	
        glUseProgram(0);
    	
        // render the screen texture
        renderShader.use();
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(2);

        glBindTexture(GL_TEXTURE_2D, screenTexture);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(0);

        glUseProgram(0);

    	
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
}

void mouse_callback(GLFWwindow*, const double xPosD, const double yPosD)
{
    const auto xPos = float(xPosD);
    const auto yPos = float(yPosD);

    if (controller.firstMouse) {
        controller.lastMousePos.x = xPos;
        controller.lastMousePos.y = yPos;
        controller.firstMouse = false;
        return; // nothing to calculate because we technically didn't move the mouse
    }

    const float xOffset = xPos - controller.lastMousePos.x;
    const float yOffset = controller.lastMousePos.y - yPos;

    controller.lastMousePos.x = xPos;
    controller.lastMousePos.y = yPos;

    cameraYaw += xOffset / 1000.0f;
    cameraPitch += yOffset / 1000.0f;

	// TODO loop yaw around
    cameraPitch = clamp(cameraPitch, -90.0f, 90.0f);
}

void key_callback(GLFWwindow*, const int key, const int scancode, const int action, const int mods)
{
    if(action == GLFW_PRESS)
    {
    	switch(key)
    	{
        case GLFW_KEY_W:
            controller.forward = 1.0f;
            break;
        case GLFW_KEY_S:
            controller.forward = -1.0f;
            break;
        case GLFW_KEY_D:
            controller.right = 1.0f;
            break;
        case GLFW_KEY_A:
            controller.right = -1.0f;
            break;
        case GLFW_KEY_SPACE:
            controller.jump = true;
    	}
    } else // action == GLFW_RELEASE
    {
        switch (key)
        {
        case GLFW_KEY_W:
        case GLFW_KEY_S:
            controller.forward = 0.0f;
            break;
        case GLFW_KEY_D:
        case GLFW_KEY_A:
            controller.right = 0.0f;
            break;
        case GLFW_KEY_SPACE:
            controller.jump = false;
        }
    }
}

void initBuffers(GLuint* vao, GLuint* buffer) {
    GLfloat vertices[] = {
        -1.f, -1.f,
        0.f, 1.f,
        -1.f, 1.f,
        0.f, 0.f,
        1.f, -1.f,
        1.f, 1.f,
        -1.f, 1.f,
        0.f, 0.f,
        1.f, -1.f,
        1.f, 1.f,
        1.f, 1.f,
        1.f, 0.f
    };

    glGenVertexArrays(1, vao);
    glBindVertexArray(*vao);

    glGenBuffers(1, buffer);
    glBindBuffer(GL_ARRAY_BUFFER, *buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initTexture(GLuint* texture, const int width, const int height) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, *texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

int main(int argc, char** argv)
{
    if (!glfwInit())
    {
        // Initialization failed
        std::cout << "Failed to init GLFW!\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // add on Mac bc Apple is big dumb :(
#endif
    
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Minecraft4k", nullptr, nullptr);
    if (!window)
    {
        // Window or OpenGL context creation failed
        std::cout << "Failed to create window!\n";
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // turn on VSync so we don't run at about a kjghpillion fps
    glfwSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD!\n" << std::endl;
        return -1;
    }

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(error_callback, nullptr);
	
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glClearDepth(1);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);	
    glCullFace(GL_FRONT_AND_BACK);
    glClearColor(0, 0, 0, 1);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, key_callback);

    renderShader = Shader("screen", "screen");
    computeProgram = loadCompute("res/raytrace.comp");

    
    glActiveTexture(GL_TEXTURE0);
    initBuffers(&vao, &buffer);
    initTexture(&screenTexture, SCR_RES_X, SCR_RES_Y);

    init();
	
    run(window);
}