#include <cmath>

#ifdef __unix__
#include <unistd.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Constants.h"
#include "Shader.h"
#include "TextureGenerator.h"
#include "Util.h"
#include "World.h"

#include "screen.vert.h"
#include "screen.frag.h"
#include "raytrace.comp.h"

struct Controller
{
    float forward;
    float right;

    bool jump;

    vec2 lastMousePos;

    bool firstMouse = true;

    void reset()
    {
        forward = 0.0f;
        right = 0.0f;
        jump = false;
    }
};

Controller controller{};

bool needsResUpdate = true;

int SCR_DETAIL = 2;

constexpr vec2 defaultRes(214, 120);

vec2 SCR_RES = defaultRes * float(1 << SCR_DETAIL);

Shader screenShader;
Shader computeShader;
GLuint buffer;
GLuint vao;

GLuint textureAtlasTex;
GLuint worldTexture;
GLuint screenTexture;

float deltaTime = 16.666f; // 16.66 = 60fps

// spawn player at world center
vec3 playerPos = vec3(WORLD_SIZE / 2.0f + 0.5f,
                      1, 
                      WORLD_SIZE / 2.0f + 0.5f);

vec3 playerVelocity;

vec3 hoveredBlockPos;
vec3 placeBlockPos;

vec3 newHoverBlockPos;

vec3 lightDirection = vec3(0.866025404f, -0.866025404f, 0.866025404f);

float cameraYaw = 0.01f; // make it not axis aligned by default to avoid raymarching error
float cameraPitch = -2.0f * PI;                                                                                 
float FOV = 90.0f;
vec2 frustumDiv = (SCR_RES * FOV);

float sinYaw, sinPitch;
float cosYaw, cosPitch;

vec3 ambColor;
vec3 skyColor;
vec3 sunColor;

uint8_t hotbar[] { BLOCK_GRASS, BLOCK_DEFAULT_DIRT, BLOCK_STONE, BLOCK_BRICKS, BLOCK_WOOD, BLOCK_LEAVES };
int heldBlockIndex = 0;

static vec3 lerp(const vec3& start, const vec3& end, const float t)
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


    char* title = new char[32];
    strcpy(title, "Minecraft4k");

    switch (SCR_DETAIL) {
    case -4:
        strcat(title, " on battery-saving mode");
        break;
    case -3:
        strcat(title, " on a potato");
        break;
    case -2:
        strcat(title, " on an undocked switch");
        break;
    case -1:
        strcat(title, " on a TI-84");
        break;
    case 0:
        strcat(title, " on an Atari 2600");
        break;
    case 2:
        strcat(title, " at SD");
        break;
    case 3:
        strcat(title, " at HD");
        break;
    case 4:
        strcat(title, " at Full HD");
        break;
    case 5:
        strcat(title, " at 4K");
        break;
    case 6:
        strcat(title, " on a NASA supercomputer");
        break;
    }

    glfwSetWindowTitle(window, title);

    glDeleteTextures(1, &screenTexture);
    initTexture(&screenTexture, int(SCR_RES.x), int(SCR_RES.y));

    needsResUpdate = false;
}

void init()
{
    // generate world

    prints("Generating world... ");
#ifdef CLASSIC
    World::generateWorld(18295169L);
#else
    World::generateWorld();
#endif

    prints("Done!\n");

    prints("Uploading world to GPU... ");
    glGenTextures(1, &worldTexture);
    glBindTexture(GL_TEXTURE_3D, worldTexture);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexStorage3D(GL_TEXTURE_3D,               // target
        1,                                      // levels
        GL_R8,                                  // internal format
        WORLD_SIZE, WORLD_HEIGHT, WORLD_SIZE);  // size

    glTexSubImage3D(GL_TEXTURE_3D,              // target
        0,                                      // level
        0, 0, 0,                                // offsets
        WORLD_SIZE, WORLD_HEIGHT, WORLD_SIZE,   // size
        GL_RED,                                 // format
        GL_UNSIGNED_BYTE,                       // type
        World::world);                          // pixels

    glBindTexture(GL_TEXTURE_3D, 0);

    prints("Done!\n");

    prints("Generating textures... ");
    textureAtlasTex = generateTextures(151910774187927L);

    prints("Finished initializing engine! Onto the game.\n");
}

void collidePlayer()
{
    // check for movement on each axis individually?
    for (int axis = 0; axis < 3; axis++) {
        bool moveValid = true;

        const vec3 newPlayerPos = vec3(playerPos.x + playerVelocity.x * (axis == 0),
                                       playerPos.y + playerVelocity.y * (axis == 1),
                                       playerPos.z + playerVelocity.z * (axis == 2));

        for (int colliderIndex = 0; colliderIndex < 12; colliderIndex++) {
            // magic
            const vec3 colliderBlockPos = vec3((newPlayerPos.x + (colliderIndex       % 2) * 0.6f - 0.3f ),
                                               (newPlayerPos.y + (colliderIndex / 4   - 1) * 0.8f + 0.65f),
                                               (newPlayerPos.z + (colliderIndex / 2   % 2) * 0.6f - 0.3f ));

            if (colliderBlockPos.y < 0) // ignore collision above the world height limit
                continue;

            // check collision with world bounds and blocks
            if (!World::isWithinWorld(colliderBlockPos)
                || World::getBlock(colliderBlockPos) != BLOCK_AIR) {

                if (axis == 1) // AXIS_Y
                {
                    // if we're falling, colliding, and we press space
                    if (controller.jump && playerVelocity.y > 0.0f) {

                        playerVelocity.y = -0.1F; // jump
                    }
                    else { // we're on the ground, not jumping

                        playerVelocity.y = 0.0f; // prevent accelerating downwards infinitely
                    }
                }

                moveValid = false;
                break;
            }
        }

        if (moveValid) {
            playerPos = newPlayerPos;
        }
    }

    //prints(playerPos); prints('\n');
}

void pollInputs(GLFWwindow* window);

void run(GLFWwindow* window) {
    auto lastUpdateTime = currentTime();
    float lastFrameTime = lastUpdateTime - 16;

    while (!glfwWindowShouldClose(window)) {
        const float frameTime = currentTime();
        deltaTime = frameTime - lastFrameTime;
        lastFrameTime = frameTime;

        pollInputs(window);

        if (needsResUpdate) {
            updateScreenResolution(window);
        }

        sinYaw = sin(cameraYaw);
        cosYaw = cos(cameraYaw);
        sinPitch = sin(cameraPitch);
        cosPitch = cos(cameraPitch);

        lightDirection.y = sin(frameTime / 10000.0f);
        lightDirection.x = lightDirection.y * 0.5f;
        lightDirection.z = cos(frameTime / 10000.0f);

        lightDirection.normalize();

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
                int magicX = int(playerPos.x +       (colliderIndex       & 1) * 0.6F - 0.3F);
                int magicY = int(playerPos.y + float((colliderIndex >> 2) - 1) * 0.8F + 0.65F);
                int magicZ = int(playerPos.z +       (colliderIndex >> 1  & 1) * 0.6F - 0.3F);

                // set block to air if inside player
                if (World::isWithinWorld(vec3(magicX, magicY, magicZ)))
                    World::setBlock(magicX, magicY, magicZ, BLOCK_AIR);
            }

            lastUpdateTime += 10;
        }

        //raycast(SCR_RES / 2.0f, hoveredBlockPos, placeBlockPos);

        //prints(hoveredBlockPos); prints("\n");


        // Compute the raytracing!
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        frustumDiv = (SCR_RES * FOV) / defaultRes;

        computeShader.use();

        glBindImageTexture(1, worldTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R8UI);
        computeShader.setVec2(PASS_STR("screenSize"), SCR_RES.x, SCR_RES.y);

        glBindTexture(GL_TEXTURE_2D, textureAtlasTex);
        computeShader.setInt(PASS_STR("textureAtlas"), 0);

        computeShader.setFloat(PASS_STR("cam.cosYaw"), cos(cameraYaw));
        computeShader.setFloat(PASS_STR("cam.cosPitch"), cos(cameraPitch));
        computeShader.setFloat(PASS_STR("cam.sinYaw"), sin(cameraYaw));
        computeShader.setFloat(PASS_STR("cam.sinPitch"), sin(cameraPitch));
        computeShader.setVec2(PASS_STR("cam.frustumDiv"), frustumDiv);
        computeShader.setVec3(PASS_STR("cam.pos"), playerPos);

#ifdef CLASSIC

#else
        computeShader.setVec3(PASS_STR("lightDirection"), lightDirection);
        computeShader.setVec3(PASS_STR("skyColor"), skyColor);
        computeShader.setVec3(PASS_STR("ambColor"), ambColor);
        computeShader.setVec3(PASS_STR("sunColor"), sunColor);
#endif

        //computeShader.setVec3("fogColor", skyColor);


        glInvalidateTexImage(screenTexture, 0);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glDispatchCompute(GLuint((SCR_RES.x + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE), GLuint((SCR_RES.y + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE), 1);
        glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
        glUseProgram(0);

        // render the screen texture
        screenShader.use();
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(2);

        glBindTexture(GL_TEXTURE_2D, screenTexture);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisableVertexAttribArray(2);
        glDisableVertexAttribArray(0);

        glUseProgram(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // put it out of its misery (evil code)
#ifdef __unix__
    _exit(0); // UNIX has a ruder function >:)
#else
    _Exit(0);
#endif

    //glfwTerminate();
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

bool keyDown(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

// TODO this ain't working
struct KeyEntry {
    int keyID;
    float keyTime;
};

std::vector<KeyEntry> keyHistory;
bool keyPress(GLFWwindow* window, int key)
{
    int pressState = glfwGetKey(window, key);

    bool keyInHistory = false;
    int loc = 0;
    for(; loc++; loc < keyHistory.size()) {
        if(keyHistory[loc].keyID == key) {
            keyInHistory = true;
            break;
        }
    }

    if(pressState == GLFW_PRESS)
    {
        if (!keyInHistory) { // not in history, this is first key press
            keyHistory.push_back({key, currentTime()});

            return true;
        }

        auto time = currentTime();
        if (time - keyHistory[loc].keyTime > 500) // repeat key press after 500ms
            return true;
        else
            return false;
    }

    if (keyInHistory) // key released, so clear from history
        keyHistory.erase(keyHistory.begin() + loc);

    return false;
}

void pollInputs(GLFWwindow* window)
{
    controller.reset();

    if(keyDown(window, GLFW_KEY_W))
        controller.forward += 1.0f;
    if (keyDown(window, GLFW_KEY_S))
        controller.forward -= 1.0f;
    if (keyDown(window, GLFW_KEY_D))
        controller.right += 1.0f;
    if (keyDown(window, GLFW_KEY_A))
        controller.right -= 1.0f;
    if (keyDown(window, GLFW_KEY_SPACE))
        controller.jump = true;

    if (keyPress(window, GLFW_KEY_COMMA)) {
        SCR_DETAIL--;
        needsResUpdate = true;
    }
    if (keyPress(window, GLFW_KEY_PERIOD)) {
        SCR_DETAIL++;
        needsResUpdate = true;
    }

    controller.forward = clamp(controller.forward, -1.0f, 1.0f);
    controller.right = clamp(controller.right, -1.0f, 1.0f);
}

void initBuffers() {
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

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initTexture(GLuint* texture, const int width, const int height) {
    glGenTextures(1, texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindImageTexture(0, *texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// TODO maybe use _start: https://int21.de/linux4k/
int main(const int argc, const char** argv)
{
    prints("Initializing GLFW... ");

    if (!glfwInit())
    {
        // Initialization failed
        prints("Failed to init GLFW!\n");
        return -1;
    }

    prints("Done!\n");

    prints("Creating window... ");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#ifdef _DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // add on Mac bc Apple is big dumb :(
#endif
    
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Minecraft4k", nullptr, nullptr);
    if (!window)
    {
        // Window or OpenGL context creation failed
        prints("Failed to create window!\n");
        return -1;
    }
    prints("Done!\n");

    prints("Setting OpenGL context... ");
    glfwMakeContextCurrent(window);
    prints("Done!\n");

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // turn on VSync so we don't run at about a kjghpillion fps
    glfwSwapInterval(1);

    prints("Loading OpenGL functions... ");
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        prints("Failed to initialize GLAD!\n");
        return -1;
    }
    prints("Done!\n");

    prints("Configuring OpenGL... ");
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

    prints("Done!\n");

    prints("Building shaders... ");
    char* numberBuf = new char[8];

    char* defines = new char[256];
    strcpy(defines, "#define WORLD_SIZE ");
    itoa(WORLD_SIZE, numberBuf); strcat(defines, numberBuf);
    strcat(defines, "\n#define WORLD_HEIGHT ");
    itoa(WORLD_HEIGHT, numberBuf); strcat(defines, numberBuf);
    strcat(defines, "\n#define TEXTURE_RES ");
    itoa(TEXTURE_RES, numberBuf); strcat(defines, numberBuf);
    strcat(defines, "\n#define RENDER_DIST ");
    itoa(RENDER_DIST, numberBuf); strcat(defines, numberBuf);
    
#ifdef CLASSIC
    defines << "\n#define CLASSIC";
#endif

    strcat(defines, "\nlayout(local_size_x = ");
    itoa(WORK_GROUP_SIZE, numberBuf); strcat(defines, numberBuf);
    strcat(defines, ", local_size_y = "); strcat(defines, numberBuf);
    strcat(defines, ") in;\n");

    screenShader = Shader(SCREEN_VERT, SCREEN_FRAG);
    computeShader = Shader(RAYTRACE_COMP);

    prints("Done!\n");
    
    glActiveTexture(GL_TEXTURE0);

    prints("Building buffers... ");
    initBuffers();
    prints("Done!\n");

    prints("Building render texture... ");
    initTexture(&screenTexture, int(SCR_RES.x), int(SCR_RES.y));
    prints("Done!\n");

    prints("Initializing engine...\n");
    init();
    prints("Finished initializing engine! Running the game...\n");

    run(window);
}