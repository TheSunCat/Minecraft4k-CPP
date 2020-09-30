#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Constants.h"
#include "Shader.h"
#include "Util.h"
#include "TextureGenerator.h"

struct Controller
{
    float forward;
    float right;

    bool jump;

    glm::vec2 lastMousePos;

    bool firstMouse = true;
};

Controller controller{};

bool needsResUpdate = true;

int SCR_DETAIL = 2;

glm::vec2 SCR_RES = glm::ivec2(107 * pow(2, SCR_DETAIL), 60 * pow(2, SCR_DETAIL));
glm::vec2 defaultRes = glm::ivec2(214, 120);

Shader screenShader;
Shader computeShader;
GLuint buffer;
GLuint vao;

GLuint textureAtlasTex;
GLuint worldTexture;
GLuint screenTexture;

float deltaTime = 16.666f; // 16.66 = 60fps

glm::vec3 playerPos = glm::vec3(WORLD_SIZE + WORLD_SIZE / 2.0f + 0.5f, 
                                WORLD_HEIGHT + 1, 
                                WORLD_SIZE + WORLD_SIZE / 2.0f + 0.5f);
glm::vec3 playerVelocity;

glm::vec3 hoveredBlockPos;
glm::vec3 placeBlockPos;

glm::vec3 newHoverBlockPos;

glm::vec3 lightDirection = glm::vec3(0.866025404f, -0.866025404f, 0.866025404f);

float cameraYaw = 0.0f;
float cameraPitch = -2.0f * PI;                                                                                 
float FOV = 90.0f;
glm::vec2 frustumDiv = (SCR_RES * FOV) / defaultRes;

float sinYaw, sinPitch;
float cosYaw, cosPitch;

glm::vec3 sunColor;
glm::vec3 ambColor;
glm::vec3 skyColor;

uint8_t world[WORLD_SIZE * WORLD_HEIGHT * WORLD_SIZE];

uint8_t hotbar[] { BLOCK_GRASS, BLOCK_DEFAULT_DIRT, BLOCK_STONE, BLOCK_BRICKS, BLOCK_WOOD, BLOCK_LEAVES };
int heldBlockIndex = 0;

bool render = true;
bool compute = true;
bool fullCompute = true;

#define getw x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_HEIGHT
static void setBlock(const int x, const int y, const int z, const uint8_t block)
{
    world[getw] = block;
}

static uint8_t getBlock(const int x, const int y, const int z)
{
    return world[getw];
}

static uint8_t getBlock(const glm::vec3& pos)
{
    return getBlock(pos.x, pos.y, pos.z);
}

static bool isWithinWorld(const glm::vec3& pos)
{
    return pos.x >= 0.0f && pos.y >= 0.0f && pos.z >= 0.0f &&
        pos.x < WORLD_SIZE&& pos.y < WORLD_HEIGHT&& pos.z < WORLD_SIZE;
}

static void fillBox(const uint8_t blockId, const glm::vec3& pos0,
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
    return start + (end - start) * t;
}

void initTexture(GLuint* texture, const int width, const int height);

void updateScreenResolution(GLFWwindow* window)
{
    if (SCR_DETAIL < -4)
        SCR_DETAIL = -4;
    if (SCR_DETAIL > 6)
        SCR_DETAIL = 6;

    SCR_RES.x = 107 * pow(2, SCR_DETAIL);
    SCR_RES.y = 60 * pow(2, SCR_DETAIL);


    std::string title = "Minecraft4k";

    switch (SCR_DETAIL) {
    case -4:
        title += " on battery-saving mode";
        break;
    case -3:
        title += " on a potato";
        break;
    case -2:
        title += " on an undocked switch";
        break;
    case -1:
        title += " on a TI-84";
        break;
    case 0:
        title += " on an Atari 2600";
        break;
    case 2:
        title += " at SD";
        break;
    case 3:
        title += " at HD";
        break;
    case 4:
        title += " at Full HD";
        break;
    case 5:
        title += " at 4K";
        break;
    case 6:
        title += " on a NASA supercomputer";
        break;
    }

    glfwSetWindowTitle(window, title.c_str());

    glDeleteTextures(1, &screenTexture);
    initTexture(&screenTexture, SCR_RES.x, SCR_RES.y);

    needsResUpdate = false;
}

void init()
{
    Random rand = Random(18295169L);

    // generate world

    std::cout << "Generating world... ";

    const float maxTerrainHeight = WORLD_HEIGHT / 2.0f;
#ifdef CLASSIC
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
#else
    const float halfWorldSize = WORLD_SIZE / 2.0f;

    constexpr int stoneDepth = 5;

    for (int x = 0; x < WORLD_SIZE; x++) {
        for (int z = 0; z < WORLD_SIZE; z++) {
            const int terrainHeight = round(maxTerrainHeight + Perlin::noise(x / halfWorldSize, z / halfWorldSize) * 10.0f);

            for (int y = terrainHeight; y < WORLD_HEIGHT; y++)
            {
                uint8_t block;

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
                const int treeX = x + (rand.nextInt(4) - 2);
                const int treeZ = z + (rand.nextInt(4) - 2);

                const int terrainHeight = round(maxTerrainHeight + Perlin::noise(treeX / halfWorldSize, treeZ / halfWorldSize) * 10.0f) - 1;

                const int treeHeight = 4 + rand.nextInt(2); // min 4 max 5

                for (int y = terrainHeight; y >= terrainHeight - treeHeight; y--)
                {
                    setBlock(treeX, y, treeZ, BLOCK_NEW);
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
#endif

    std::cout << "Done!\n";

    std::cout << "Generating textures... ";
    textureAtlasTex = generateTextures();
    std::cout << "Done!\n";

    std::cout << "Uploading world data... ";
    glGenTextures(1, &worldTexture);
    glBindTexture(GL_TEXTURE_3D, worldTexture);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RED, WORLD_SIZE, WORLD_HEIGHT, WORLD_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, world);
    glBindTexture(GL_TEXTURE_3D, 0);

    std::cout << "Done!\n";
}

void raycast(const glm::vec2 pixelCoords, glm::vec3& hoveredBlockPos_out, glm::vec3& placeBlockPos_out)
{
    // rotate frustum space to world space
    const float temp = cosPitch + sinPitch;

    glm::vec3 rayDir = glm::normalize(glm::vec3(cosYaw + temp * sinYaw,
                                                cosPitch - sinPitch,
                                                temp * cosYaw - sinYaw));

    float furthestHit = PLAYER_REACH;
    glm::vec3 closestHit(-1);
    int closestHitAxis = 0;
    int closestHitDir = 0;

    glm::vec3 rayOrigin = playerPos;

    for (int axis = 0; axis < 3; axis++)
    {
        // align ray to block edge on this axis
        // and calc ray deltas
        const float delta = rayDir[axis];

        const glm::vec3 rayDelta = rayDir / abs(delta);

        const float playerOffsetFromBlockEdge = delta > 0 ? 1.0f - glm::fract(rayOrigin[axis]) : glm::fract(rayOrigin[axis]);

        glm::vec3 rayPos = rayOrigin + rayDelta * playerOffsetFromBlockEdge;
        rayPos[axis] -= delta < 0 ? 1 : 0;

        float rayTravelDist = playerOffsetFromBlockEdge / abs(delta);

        // do the raycast
        while (rayTravelDist < furthestHit)
        {
            const glm::vec3 blockHit = glm::vec3(rayPos.x - WORLD_SIZE, rayPos.y - WORLD_HEIGHT, rayPos.z - WORLD_SIZE);

            // if ray exits the world
            if (!isWithinWorld(blockHit))
                break;

            const int blockHitID = blockHit.y < 0 ? BLOCK_AIR : getBlock(blockHit);                                                                        

            if (blockHitID != BLOCK_AIR)
            {
                closestHit = blockHit;
                closestHitAxis = axis;
                closestHitDir = glm::sign(rayDelta[closestHitAxis]);

                furthestHit = rayTravelDist;
            }

            rayPos += rayDelta;
            rayTravelDist += 1.0f / abs(delta);
        }
    }

    hoveredBlockPos_out = closestHit;

    placeBlockPos_out = closestHit;
    hoveredBlockPos_out[abs(closestHitDir)] += closestHitDir;
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
    long long lastTime = currentTime() - 16;
    long long lastUpdateTime = currentTime();

    while (!glfwWindowShouldClose(window)) {
        const long long startTime = currentTime();
        deltaTime = startTime - lastTime;
        lastTime = startTime;


        if (needsResUpdate) {
            updateScreenResolution(window);
        }

        sinYaw = sin(cameraYaw);
        cosYaw = cos(cameraYaw);
        sinPitch = sin(cameraPitch);
        cosPitch = cos(cameraPitch);

        lightDirection.y = sin(startTime / 10000.0);
        lightDirection.x = lightDirection.y * 0.5f;
        lightDirection.z = cos(startTime / 10000.0);

        lightDirection = glm::normalize(lightDirection);


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
        
        while (currentTime() - lastUpdateTime > 10)
        {
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
                int magicX = int(playerPos.x + (colliderIndex & 1) * 0.6F - 0.3F) - WORLD_SIZE;
                int magicY = int(playerPos.y + ((colliderIndex >> 2) - 1) * 0.8F + 0.65F) - WORLD_HEIGHT;
                int magicZ = int(playerPos.z + (colliderIndex >> 1 & 1) * 0.6F - 0.3F) - WORLD_SIZE;

                // set block to air if inside player
                if (isWithinWorld(glm::vec3(magicX, magicY, magicZ)))
                    setBlock(magicX, magicY, magicZ, BLOCK_AIR);
            }

            lastUpdateTime += 10;
        }

        raycast(SCR_RES / 2.0f, hoveredBlockPos, placeBlockPos);

        std::cout << hoveredBlockPos << "\n";

        if (render)
        {
            // Compute the raytracing!
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (compute)
            {
                frustumDiv = (SCR_RES * FOV) / defaultRes;

                computeShader.use();

                glBindImageTexture(1, worldTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8UI);
                computeShader.setVec2("screenSize", SCR_RES.x, SCR_RES.y);

                glBindTexture(GL_TEXTURE_2D, textureAtlasTex);
                computeShader.setInt("textureAtlas", 0);

                computeShader.setFloat("camera.cosYaw", cos(cameraYaw));
                computeShader.setFloat("camera.cosPitch", cos(cameraPitch));
                computeShader.setFloat("camera.sinYaw", sin(cameraYaw));
                computeShader.setFloat("camera.sinPitch", sin(cameraPitch));

                computeShader.setVec2("frustumDiv", frustumDiv);

                computeShader.setVec3("cameraPos", playerPos);

                computeShader.setVec3("hoveredBlockPos", hoveredBlockPos);

                computeShader.setVec3("lightDirection", lightDirection);
                computeShader.setVec3("sunColor", sunColor);
                computeShader.setVec3("ambColor", ambColor);
                computeShader.setVec3("skyColor", skyColor);

                glInvalidateTexImage(screenTexture, 0);

                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                glDispatchCompute((SCR_RES.x + 15) / 16, (SCR_RES.y + 15) / 16, 1);
                glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
                glUseProgram(0);
            }

            // render the screen texture
            screenShader.use();
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
        }

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

    cameraYaw += xOffset / 500.0f;
    cameraPitch += yOffset / 500.0f;

    if(fabs(cameraYaw) > PI)
    {
        if (cameraYaw > 0)
            cameraYaw = -PI - (cameraYaw - PI);
        else
            cameraYaw = PI + (cameraYaw + PI);
    }
    cameraPitch = clamp(cameraPitch, -PI / 2.0f, PI / 2.0f);
}

void key_callback(GLFWwindow*, const int key, const int scancode, const int action, const int mods)
{
    if(action == GLFW_PRESS || action == GLFW_REPEAT)
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
            break;
        case GLFW_KEY_COMMA:
            SCR_DETAIL--;
            needsResUpdate = true;
            break;
        case GLFW_KEY_PERIOD:
            SCR_DETAIL++;
            needsResUpdate = true;
            break;
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
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, *texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

int main(const int argc, const char** argv)
{
    if(argc == 2)
    {
        std::string argument = argv[1];
        switch(argument[0])
        {
        case 'a':
            render = false;
        case 's':
            compute = false;
        case 'd':
            fullCompute = false;
        }
    }

    std::cout << "Initializing GLFW... ";

    if (!glfwInit())
    {
        // Initialization failed
        std::cout << "Failed to init GLFW!\n";
        return -1;
    }

    std::cout << "Done!\n";

    std::cout << "Creating window... ";
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
    std::cout << "Done!\n";

    std::cout << "Setting OpenGL context... ";
    glfwMakeContextCurrent(window);
    std::cout << "Done!\n";

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // turn on VSync so we don't run at about a kjghpillion fps
    glfwSwapInterval(1);

    std::cout << "Loading OpenGL functions... ";
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD!\n" << std::endl;
        return -1;
    }
    std::cout << "Done!\n";

    std::cout << "Configuring OpenGL... ";
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

    std::cout << "Done!\n";

    std::cout << "Building shaders... ";
    std::stringstream defines;
    defines << "#define WORLD_SIZE " << WORLD_SIZE << "\n"
            << "#define WORLD_HEIGHT " << WORLD_HEIGHT << "\n"
            << "#define TEXTURE_RES " << TEXTURE_RES << "\n"
            << "#define RENDER_DIST " << RENDER_DIST << "\n";
    const std::string definesStr = defines.str();

    screenShader = Shader("screen", "screen");

    if (fullCompute)
        computeShader = Shader("raytrace", HasExtra::Yes, definesStr.c_str());
    else
        computeShader = Shader(HasExtra::No, 
       "#version 430\n"
            "layout(local_size_x = 1, local_size_y = 1) in;\n"
            "layout(rgba32f, binding = 0) uniform image2D img_output;\n"
            "\n"
            "void main() {\n"
                "imageStore(img_output, ivec2(gl_GlobalInvocationID.xy), vec4(0.5));\n"
            "}");
    std::cout << "Done!\n";
    
    glActiveTexture(GL_TEXTURE0);

    std::cout << "Building buffers... ";
    initBuffers(&vao, &buffer);
    std::cout << "Done!\n";

    std::cout << "Building render texture... ";
    initTexture(&screenTexture, SCR_RES.x, SCR_RES.y);
    std::cout << "Done!\n";

    std::cout << "Initializing engine...\n";
    init();
    std::cout << "Finished initializing engine! Running the game...\n";

    run(window);
}