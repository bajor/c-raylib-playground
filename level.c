#include "level.h"
#include <stdio.h>

Color levelData[LEVEL_W * LEVEL_H];

static inline bool IsWallColor(Color c) { return c.r == 0 && c.g == 0 && c.b == 0; }

bool IsWall(Color c) { return IsWallColor(c); }

bool LoadLevel(const char *imagePath)
{
    Image img = LoadImage(imagePath);
    if (img.width != LEVEL_W || img.height != LEVEL_H) {
        printf("Level image must be %dx%d\n", LEVEL_W, LEVEL_H);
        UnloadImage(img);
        return false;
    }
    Color *pixels = LoadImageColors(img);
    for (int i = 0; i < LEVEL_W * LEVEL_H; ++i) levelData[i] = pixels[i];
    UnloadImageColors(pixels);
    UnloadImage(img);
    return true;
}

bool CollidesWithLevel(Rectangle r)
{
    int minX = (int)r.x / TILE_SIZE;
    int minY = (int)r.y / TILE_SIZE;
    int maxX = (int)(r.x + r.width ) / TILE_SIZE;
    int maxY = (int)(r.y + r.height) / TILE_SIZE;

    minX = (minX < 0) ? 0 : minX;
    minY = (minY < 0) ? 0 : minY;
    maxX = (maxX >= LEVEL_W) ? LEVEL_W - 1 : maxX;
    maxY = (maxY >= LEVEL_H) ? LEVEL_H - 1 : maxY;

    for (int y = minY; y <= maxY; ++y)
        for (int x = minX; x <= maxX; ++x)
            if (IsWallColor(levelData[y * LEVEL_W + x]) &&
                CheckCollisionRecs(r, (Rectangle){x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE}))
                return true;

    return false;
}

void DrawLevel(void)
{
    for (int y = 0; y < LEVEL_H; ++y)
        for (int x = 0; x < LEVEL_W; ++x)
            if (IsWallColor(levelData[y * LEVEL_W + x]))
                DrawRectangle(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE, DARKGRAY);
}
