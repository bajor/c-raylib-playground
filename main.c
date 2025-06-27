#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define SCREEN_W 800
#define SCREEN_H 450
#define TILE_SIZE 25
#define LEVEL_W 32
#define LEVEL_H 18
#define PLAYER_SIZE 18
#define PLAYER_SPEED 3
#define MAX_DOTS 10

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

// Red dot struct
typedef struct {
    Rectangle rect;
    int alive;
    Vector2 last_pos;
    int stuck_frames;
} RedDot;

int main(void) {
    if (!FileExists("level.png")) GenerateLevelPNG("level.png");
    Image levelImg = LoadImage("level.png");
    Color *levelData = LoadImageColors(levelImg);
    InitWindow(SCREEN_W, SCREEN_H, "Top-down Platformer – raylib");
    SetTargetFPS(60);

    Texture2D pug = LoadTexture("pug.png");

    Rectangle player = {TILE_SIZE + 4, TILE_SIZE + 4, PLAYER_SIZE, PLAYER_SIZE};
    Vector2 velocity = {0, 0};

    // Red dots: spawn on walkable tiles, not too close to player
    RedDot dots[MAX_DOTS];
    int dots_spawned = 0;
    while (dots_spawned < MAX_DOTS) {
        int ax = rand() % LEVEL_W;
        int ay = rand() % LEVEL_H;
        float dist = sqrtf((ax-player.x)*(ax-player.x) + (ay-player.y)*(ay-player.y));
        if (levelData[ay*LEVEL_W + ax].r == 255 && dist > LEVEL_W/8) {
            dots[dots_spawned].rect = (Rectangle){ax*TILE_SIZE+TILE_SIZE/4, ay*TILE_SIZE+TILE_SIZE/4, TILE_SIZE/2, TILE_SIZE/2};
            dots[dots_spawned].alive = 1;
            dots[dots_spawned].last_pos = (Vector2){dots[dots_spawned].rect.x, dots[dots_spawned].rect.y};
            dots[dots_spawned].stuck_frames = 0;
            dots_spawned++;
        }
    }
    int score = 0;

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

        // --- Red dot AI movement ---
        for (int i = 0; i < MAX_DOTS; i++) {
            if (!dots[i].alive) continue;
            // Check if stuck (not moved much for several frames)
            float moved = sqrtf((dots[i].rect.x - dots[i].last_pos.x)*(dots[i].rect.x - dots[i].last_pos.x) + (dots[i].rect.y - dots[i].last_pos.y)*(dots[i].rect.y - dots[i].last_pos.y));
            if (moved < 1.0f) dots[i].stuck_frames++;
            else dots[i].stuck_frames = 0;
            dots[i].last_pos = (Vector2){dots[i].rect.x, dots[i].rect.y};
            // Chance for random move: base 10%, or 50% if stuck for 10+ frames
            float random_chance = 0.1f;
            if (dots[i].stuck_frames > 10) random_chance = 0.5f;
            int do_random = ((float)rand()/(float)RAND_MAX) < random_chance;
            float best_score = -1e9f;
            Vector2 best_move = {0, 0};
            Vector2 valid_moves[8];
            int valid_count = 0;
            // Try 8 directions (N, NE, E, SE, S, SW, W, NW)
            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    Rectangle test = dots[i].rect;
                    test.x += dx * PLAYER_SPEED;
                    test.y += dy * PLAYER_SPEED;
                    // Check collision with walls
                    int blocked = 0;
                    for (int y = 0; y < LEVEL_H && !blocked; y++) {
                        for (int x = 0; x < LEVEL_W && !blocked; x++) {
                            if (levelData[y*LEVEL_W + x].r == 0) {
                                Rectangle wall = {x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE};
                                if (CheckCollisionRecs(test, wall)) blocked = 1;
                            }
                        }
                    }
                    if (blocked) continue;
                    // Store valid moves for possible random selection
                    valid_moves[valid_count++] = (Vector2){dx * PLAYER_SPEED, dy * PLAYER_SPEED};
                    // Score: maximize distance from player, minimize proximity to other dots
                    float dist_player = sqrtf((test.x-player.x)*(test.x-player.x) + (test.y-player.y)*(test.y-player.y));
                    float min_dist_dot = 1e9f;
                    for (int j = 0; j < MAX_DOTS; j++) {
                        if (i == j || !dots[j].alive) continue;
                        float d = sqrtf((test.x-dots[j].rect.x)*(test.x-dots[j].rect.x) + (test.y-dots[j].rect.y)*(test.y-dots[j].rect.y));
                        if (d < min_dist_dot) min_dist_dot = d;
                    }
                    float score = dist_player + 0.7f * min_dist_dot; // weight for spreading
                    if (score > best_score) {
                        best_score = score;
                        best_move = (Vector2){dx * PLAYER_SPEED, dy * PLAYER_SPEED};
                    }
                }
            }
            // Pick random valid move if needed
            if (do_random && valid_count > 0) {
                int idx = rand() % valid_count;
                best_move = valid_moves[idx];
            }
            // Move in the best direction
            dots[i].rect.x += best_move.x;
            dots[i].rect.y += best_move.y;
        }

        // --- Catching red dots ---
        for (int i = 0; i < MAX_DOTS; i++) {
            if (dots[i].alive && CheckCollisionRecs(player, dots[i].rect)) {
                dots[i].alive = 0;
                score++;
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
        // Draw red dots
        for (int i = 0; i < MAX_DOTS; i++) {
            if (dots[i].alive) {
                DrawCircle((int)(dots[i].rect.x + dots[i].rect.width/2), (int)(dots[i].rect.y + dots[i].rect.height/2), dots[i].rect.width/2, RED);
            }
        }
        // Draw score
        DrawText(TextFormat("Score: %d", score), player.x - SCREEN_W/2 + 10, player.y - SCREEN_H/2 + 10, 24, RED);
        EndDrawing();
    }
    UnloadImageColors(levelData);
    UnloadImage(levelImg);
    UnloadTexture(pug);
    CloseWindow();
    return 0;
}
