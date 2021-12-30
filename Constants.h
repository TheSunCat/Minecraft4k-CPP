#pragma once

typedef unsigned char uint8_t;
typedef unsigned long uint64_t;

#include "Vector.h"

//#define CLASSIC

// PERFORMANCE OPTIONS

constexpr int WORK_GROUP_SIZE = 16;

constexpr int WINDOW_WIDTH = 856;
constexpr int WINDOW_HEIGHT = 480;

#ifdef CLASSIC
constexpr int RENDER_DIST = 20;
#else
constexpr int RENDER_DIST = 80;
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
constexpr vec3 SC_DAY = vec3(1);
constexpr vec3 AC_DAY = vec3(0.5f, 0.5f, 0.5f);
constexpr vec3 YC_DAY = vec3(0.317f, 0.729f, 0.969f);

constexpr vec3 SC_TWILIGHT = vec3(1, 0.5f, 0.01f);
constexpr vec3 AC_TWILIGHT = vec3(0.6f, 0.5f, 0.5f);
constexpr vec3 YC_TWILIGHT = vec3(0.27f, 0.24f, 0.33f);

constexpr vec3 SC_NIGHT = vec3(0.3f, 0.3f, 0.5f);
constexpr vec3 AC_NIGHT = vec3(0.3f, 0.3f, 0.5f);
constexpr vec3 YC_NIGHT = vec3(0.004f, 0.004f, 0.008f);