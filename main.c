#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define SCREEN_W 800
#define SCREEN_H 450
#define TILE_SIZE 25
#define LEVEL_W 32
#define LEVEL_H 18
#define PLAYER_SIZE 18
#define PLAYER_SPEED 3

// Generate a simple PNG level if not present
void GenerateLevelPNG(const char *filename) {
    Image img = GenImageColor(LEVEL_W, LEVEL_H, WHITE);
    Color *pixels = LoadImageColors(img);
    // Draw border walls
    for (int y = 0; y < LEVEL_H; y++) {
        for (int x = 0; x < LEVEL_W; x++) {
            if (x == 0 || y == 0 || x == LEVEL_W-1 || y == LEVEL_H-1) {
                pixels[y*LEVEL_W + x] = BLACK;
            }
        }
    }
    // Add some inner walls
    for (int x = 4; x < 28; x++) pixels[7*LEVEL_W + x] = BLACK;
    for (int y = 10; y < 15; y++) pixels[y*LEVEL_W + 10] = BLACK;
    for (int x = 15; x < 25; x++) pixels[13*LEVEL_W + x] = BLACK;
    memcpy(img.data, pixels, LEVEL_W * LEVEL_H * sizeof(Color));
    ExportImage(img, filename);
    UnloadImage(img);
    UnloadImageColors(pixels);
}

int main(void) {
    if (!FileExists("level.png")) GenerateLevelPNG("level.png");
    Image levelImg = LoadImage("level.png");
    Color *levelData = LoadImageColors(levelImg);
    InitWindow(SCREEN_W, SCREEN_H, "Top-down Platformer – raylib");
    SetTargetFPS(60);

    Texture2D pug = LoadTexture("pug.png");

    Rectangle player = {TILE_SIZE + 4, TILE_SIZE + 4, PLAYER_SIZE, PLAYER_SIZE};
    Vector2 velocity = {0, 0};

    while (!WindowShouldClose()) {
        // Input
        velocity = (Vector2){0, 0};
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) velocity.y = -PLAYER_SPEED;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) velocity.y = PLAYER_SPEED;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) velocity.x = -PLAYER_SPEED;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) velocity.x = PLAYER_SPEED;

        // Try to move player, check collision with walls
        Rectangle nextPos = player;
        nextPos.x += velocity.x;
        nextPos.y += velocity.y;
        bool collided = false;
        for (int y = 0; y < LEVEL_H; y++) {
            for (int x = 0; x < LEVEL_W; x++) {
                if (levelData[y*LEVEL_W + x].r == 0 && levelData[y*LEVEL_W + x].g == 0 && levelData[y*LEVEL_W + x].b == 0) {
                    Rectangle wall = {x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE};
                    if (CheckCollisionRecs(nextPos, wall)) collided = true;
                }
            }
        }
        if (!collided) {
            player = nextPos;
        } else {
            // Try X only
            nextPos = player;
            nextPos.x += velocity.x;
            collided = false;
            for (int y = 0; y < LEVEL_H; y++) {
                for (int x = 0; x < LEVEL_W; x++) {
                    if (levelData[y*LEVEL_W + x].r == 0 && levelData[y*LEVEL_W + x].g == 0 && levelData[y*LEVEL_W + x].b == 0) {
                        Rectangle wall = {x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE};
                        if (CheckCollisionRecs(nextPos, wall)) collided = true;
                    }
                }
            }
            if (!collided) {
                player = nextPos;
            } else {
                // Try Y only
                nextPos = player;
                nextPos.y += velocity.y;
                collided = false;
                for (int y = 0; y < LEVEL_H; y++) {
                    for (int x = 0; x < LEVEL_W; x++) {
                        if (levelData[y*LEVEL_W + x].r == 0 && levelData[y*LEVEL_W + x].g == 0 && levelData[y*LEVEL_W + x].b == 0) {
                            Rectangle wall = {x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE};
                            if (CheckCollisionRecs(nextPos, wall)) collided = true;
                        }
                    }
                }
                if (!collided) player = nextPos;
            }
        }

        // Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);
        // Draw level
        for (int y = 0; y < LEVEL_H; y++) {
            for (int x = 0; x < LEVEL_W; x++) {
                if (levelData[y*LEVEL_W + x].r == 0 && levelData[y*LEVEL_W + x].g == 0 && levelData[y*LEVEL_W + x].b == 0) {
                    DrawRectangle(x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE, DARKGRAY);
                }
            }
        }
        // Draw player
        DrawTexturePro(
            pug,
            (Rectangle){0, 0, pug.width, pug.height},
            (Rectangle){player.x + player.width/2 - pug.width/2, player.y + player.height/2 - pug.height/2, pug.width, pug.height},
            (Vector2){0, 0},
            0.0f,
            WHITE
        );
        EndDrawing();
    }
    UnloadImageColors(levelData);
    UnloadImage(levelImg);
    UnloadTexture(pug);
    CloseWindow();
    return 0;
}
