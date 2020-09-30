#include "TextureGenerator.h"

#include "Constants.h"
#include "Util.h"

GLuint generateTextures()
{
    // set random seed to generate textures
    Random rand = Random(151910774187927L);

    std::cout << "Building textures... ";
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

#ifndef CLASSIC
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
                const float pNoise = Perlin::noise(x, y);

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

                    const int woodCenter = TEXTURE_RES / 2 - 1;
                    int dx = x - woodCenter;
                    int dy = (y % TEXTURE_RES) - woodCenter;

                    if (dx < 0)
                        dx = 1 - dx;

                    if (dy < 0)
                        dy = 1 - dy;

                    if (dy > dx)
                        dx = dy;

                    const double distFromCenter = (sqrt(dx * dx + dy * dy) * .25f + std::max(dx, dy) * .75f);

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
                    (tint >> 8 & 0xFF) * gsd_constexpr / 0xFF << 8 |
                    (tint & 0xFF) * gsd_constexpr / 0xFF << 0;

                // write pixel to the texture atlas
                textureAtlas[x + y * TEXTURE_RES + blockType * (TEXTURE_RES * TEXTURE_RES) * 3] = col;
            }
        }
    }

    GLuint textureAtlasTex = 0;

    std::cout << "Uploading texture atlas... ";
    glGenTextures(1, &textureAtlasTex);
    glBindTexture(GL_TEXTURE_2D, textureAtlasTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, TEXTURE_RES * 3, TEXTURE_RES * 16);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXTURE_RES * 3, TEXTURE_RES * 16, GL_BGRA, GL_UNSIGNED_BYTE, textureAtlas);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] textureAtlas;
    std::cout << "Done!\n";

    return textureAtlasTex;
}
