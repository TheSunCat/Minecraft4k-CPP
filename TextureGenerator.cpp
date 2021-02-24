#include "TextureGenerator.h"

#include "Constants.h"
#include "Util.h"

GLuint generateTextures(long long seed)
{
    // set random seed to generate textures
    Random rand = Random(seed);

    std::cout << "Building textures... ";
    int* textureAtlas = new int[TEXTURE_RES * TEXTURE_RES * 3 * 16];

    // procedurally generates the 16x3 textureAtlas
    // gsd = grayscale detail
    for (int blockID = 1; blockID < 16; blockID++) {
        int gsd_tempA = 0xFF - rand.nextInt(0x60);

        for (int y = 0; y < TEXTURE_RES * 3; y++) {
            for (int x = 0; x < TEXTURE_RES; x++) {
                // gets executed per pixel/texel

                if (blockID != BLOCK_STONE || rand.nextInt(3) == 0) // if the block type is stone, update the noise value less often to get a stretched out look
                    gsd_tempA = 0xFF - rand.nextInt(0x60);

                int tint = 0x966C4A; // brown (dirt)
                switch (blockID)
                {
                case BLOCK_STONE:
                {
                    tint = 0x7F7F7F; // grey
                    break;
                }
                case BLOCK_GRASS:
                {
                    if (y < ((x * x * 3 + x * 81) >> 2 & 0x3) + (TEXTURE_RES * 1.125f)) // grass + grass edge
                        tint = 0x6AAA40; // green
                    else if (y < ((x * x * 3 + x * 81) >> 2 & 0x3) + (TEXTURE_RES * 1.1875f)) // grass edge shadow
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

                int gsd_constexpr = gsd_tempA;
                if (y >= TEXTURE_RES * 2) // bottom side of the block
                    gsd_constexpr /= 2; // make it darker, baked "shading"

                if (blockID == BLOCK_LEAVES) {
                    tint = 0x50D937; // green
                    if (rand.nextInt(2) == 0) {
                        tint = 0;
                        gsd_constexpr = 0xFF;
                    }
                }

                // multiply tint by the grayscale detail
                const int col = ((tint & 0xFFFFFF) == 0 ? 0 : 0xFF) << 24 |
                    (tint >> 16 & 0xFF) * gsd_constexpr / 0xFF << 16 |
                    (tint >> 8 & 0xFF) * gsd_constexpr / 0xFF << 8 |
                    (tint & 0xFF) * gsd_constexpr / 0xFF << 0;

                // write pixel to the texture atlas
                textureAtlas[x + (TEXTURE_RES * blockID) + y * (TEXTURE_RES * 16)] = col;
            }
        }
    }

    GLuint textureAtlasTex = 0;

    std::cout << "Uploading texture atlas to GPU... ";
    glGenTextures(1, &textureAtlasTex);
    glBindTexture(GL_TEXTURE_2D, textureAtlasTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, TEXTURE_RES * 16, TEXTURE_RES * 3);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXTURE_RES * 16, TEXTURE_RES * 3, GL_BGRA, GL_UNSIGNED_BYTE, textureAtlas);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete[] textureAtlas;

    std::cout << "Done!\n";

    return textureAtlasTex;
}
