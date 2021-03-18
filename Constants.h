#pragma once
#include <cstdint>
#include "include/glm/vec3.hpp"

//#define CLASSIC


// PERFORMANCE OPTIONS

constexpr int WORK_GROUP_SIZE = 16;

constexpr int WINDOW_WIDTH = 856;
constexpr int WINDOW_HEIGHT = 480;

#ifdef CLASSIC
constexpr float RENDER_DIST = 20.0f;
#else
constexpr float RENDER_DIST = 80.0f;
#endif


constexpr int TEXTURE_RES = 16;

#ifdef CLASSIC
constexpr int WORLD_SIZE = 64;
#else
constexpr int WORLD_SIZE = 512;
#endif
constexpr int WORLD_HEIGHT = 64;

// END OF PERFORMANCE OPTIONS


constexpr uint8_t BLOCK_AIR = 0;
constexpr uint8_t BLOCK_GRASS = 1;
constexpr uint8_t BLOCK_DEFAULT_DIRT = 2;
constexpr uint8_t BLOCK_STONE = 4;
constexpr uint8_t BLOCK_BRICKS = 5;
constexpr uint8_t BLOCK_WOOD = 7;
constexpr uint8_t BLOCK_LEAVES = 8;
constexpr uint8_t BLOCK_MIRROR = 9;

constexpr float PLAYER_REACH = 5.0f;

// COLORS

// S = Sun, A = Amb, Y = skY
constexpr glm::vec3 SC_DAY = glm::vec3(1);
constexpr glm::vec3 AC_DAY = glm::vec3(0.5f, 0.5f, 0.5f);
constexpr glm::vec3 YC_DAY = glm::vec3(0.317f, 0.729f, 0.969f);

constexpr glm::vec3 SC_TWILIGHT = glm::vec3(1, 0.5f, 0.01f);
constexpr glm::vec3 AC_TWILIGHT = glm::vec3(0.6f, 0.5f, 0.5f);
constexpr glm::vec3 YC_TWILIGHT = glm::vec3(0.27f, 0.24f, 0.33f);

constexpr glm::vec3 SC_NIGHT = glm::vec3(0.3f, 0.3f, 0.5f);
constexpr glm::vec3 AC_NIGHT = glm::vec3(0.3f, 0.3f, 0.5f);
constexpr glm::vec3 YC_NIGHT = glm::vec3(0.004f, 0.004f, 0.008f);