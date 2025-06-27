#include "raylib.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define SCREEN_W 800
#define SCREEN_H 450
#define TILE_SIZE 25
#define LEVEL_W 32
#define LEVEL_H 18
#define PLAYER_SIZE 18
#define PLAYER_SPEED 3
#define DOT_SPEED 3.6f
#define MAX_DOTS 10

// --- Level data array ---
Color levelData[LEVEL_W * LEVEL_H];

// Red dot struct
typedef struct {
    Rectangle rect;
    int alive;
    Vector2 last_pos;
    int stuck_frames;
} RedDot;

// Helper: check if pixel is a wall (black)
static inline bool IsWall(Color c) {
    return (c.r == 0 && c.g == 0 && c.b == 0);
}

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "Top-down Platformer – raylib");
    SetTargetFPS(60);

    Texture2D pug = LoadTexture("pug.png");

    Rectangle player = {TILE_SIZE + 4, TILE_SIZE + 4, PLAYER_SIZE, PLAYER_SIZE};
    Vector2 velocity = {0, 0};

    // --- Load level data from image ---
    Image levelImg = LoadImage("level.png");
    if (levelImg.width != LEVEL_W || levelImg.height != LEVEL_H) {
        printf("level.png must be %dx%d pixels!\n", LEVEL_W, LEVEL_H);
        UnloadImage(levelImg);
        CloseWindow();
        return 1;
    }
    Color *pixels = LoadImageColors(levelImg);
    for (int i = 0; i < LEVEL_W * LEVEL_H; i++) {
        levelData[i] = pixels[i];
    }
    UnloadImageColors(pixels);
    UnloadImage(levelImg);

    // Red dots: spawn only on walkable tiles, not too close to player
    RedDot dots[MAX_DOTS] = {0};
    int dots_spawned = 0;
    while (dots_spawned < MAX_DOTS) {
        int ax = rand() % LEVEL_W;
        int ay = rand() % LEVEL_H;

        // Skip walls
        if (IsWall(levelData[ay * LEVEL_W + ax])) continue;

        // Compute distance to player in pixels
        float dx = ax * TILE_SIZE + TILE_SIZE / 2.0f - (player.x + PLAYER_SIZE / 2.0f);
        float dy = ay * TILE_SIZE + TILE_SIZE / 2.0f - (player.y + PLAYER_SIZE / 2.0f);
        float dist = sqrtf(dx * dx + dy * dy);

        if (dist < TILE_SIZE * 4) continue; // keep a minimum distance

        dots[dots_spawned].rect = (Rectangle){
            ax * TILE_SIZE + TILE_SIZE / 4.0f,
            ay * TILE_SIZE + TILE_SIZE / 4.0f,
            TILE_SIZE / 2.0f, TILE_SIZE / 2.0f
        };
        dots[dots_spawned].alive = 1;
        dots[dots_spawned].last_pos = (Vector2){dots[dots_spawned].rect.x, dots[dots_spawned].rect.y};
        dots[dots_spawned].stuck_frames = 0;
        dots_spawned++;
    }

    int score = 0;

    while (!WindowShouldClose()) {
        // Input
        velocity = (Vector2){0, 0};
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) velocity.y = -PLAYER_SPEED;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) velocity.y =  PLAYER_SPEED;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) velocity.x = -PLAYER_SPEED;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) velocity.x =  PLAYER_SPEED;

        // Try to move player, check collision with walls
        Rectangle nextPos = player;
        nextPos.x += velocity.x;
        nextPos.y += velocity.y;
        bool collided = false;
        for (int y = 0; y < LEVEL_H && !collided; y++) {
            for (int x = 0; x < LEVEL_W && !collided; x++) {
                if (IsWall(levelData[y * LEVEL_W + x])) {
                    Rectangle wall = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
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
            for (int y = 0; y < LEVEL_H && !collided; y++) {
                for (int x = 0; x < LEVEL_W && !collided; x++) {
                    if (IsWall(levelData[y * LEVEL_W + x])) {
                        Rectangle wall = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
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
                for (int y = 0; y < LEVEL_H && !collided; y++) {
                    for (int x = 0; x < LEVEL_W && !collided; x++) {
                        if (IsWall(levelData[y * LEVEL_W + x])) {
                            Rectangle wall = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
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

            float moved = sqrtf((dots[i].rect.x - dots[i].last_pos.x) *
                                    (dots[i].rect.x - dots[i].last_pos.x) +
                                (dots[i].rect.y - dots[i].last_pos.y) *
                                    (dots[i].rect.y - dots[i].last_pos.y));
            if (moved < 1.0f) dots[i].stuck_frames++;
            else dots[i].stuck_frames = 0;
            dots[i].last_pos = (Vector2){dots[i].rect.x, dots[i].rect.y};

            float random_chance = (dots[i].stuck_frames > 10) ? 0.5f : 0.1f;
            float dist_to_player = sqrtf((dots[i].rect.x - player.x) * (dots[i].rect.x - player.x) +
                                         (dots[i].rect.y - player.y) * (dots[i].rect.y - player.y));
            int force_random = (dots[i].stuck_frames > 10 && dist_to_player < 200.0f);
            int do_random = force_random || (((float)rand() / (float)RAND_MAX) < random_chance);

            float best_score = -1e9f;
            Vector2 best_move = {0, 0};
            Vector2 valid_moves[8];
            int valid_count = 0;

            for (int dx = -1; dx <= 1; dx++) {
                for (int dy = -1; dy <= 1; dy++) {
                    if (dx == 0 && dy == 0) continue;
                    Rectangle test = dots[i].rect;
                    test.x += dx * DOT_SPEED;
                    test.y += dy * DOT_SPEED;

                    int blocked = 0;
                    for (int y = 0; y < LEVEL_H && !blocked; y++) {
                        for (int x = 0; x < LEVEL_W && !blocked; x++) {
                            if (IsWall(levelData[y * LEVEL_W + x])) {
                                Rectangle wall = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
                                if (CheckCollisionRecs(test, wall)) blocked = 1;
                            }
                        }
                    }
                    if (blocked) continue;

                    valid_moves[valid_count++] = (Vector2){dx * DOT_SPEED, dy * DOT_SPEED};

                    float dist_player = sqrtf((test.x - player.x) * (test.x - player.x) +
                                              (test.y - player.y) * (test.y - player.y));

                    float min_dist_dot = 1e9f;
                    for (int j = 0; j < MAX_DOTS; j++) {
                        if (i == j || !dots[j].alive) continue;
                        float d = sqrtf((test.x - dots[j].rect.x) * (test.x - dots[j].rect.x) +
                                        (test.y - dots[j].rect.y) * (test.y - dots[j].rect.y));
                        if (d < min_dist_dot) min_dist_dot = d;
                    }
                    float score = dist_player + 0.7f * min_dist_dot; // weight for spreading
                    if (score > best_score) {
                        best_score = score;
                        best_move = (Vector2){dx * DOT_SPEED, dy * DOT_SPEED};
                    }
                }
            }

            if (do_random && valid_count > 0) {
                best_move = valid_moves[rand() % valid_count];
            }

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
                if (IsWall(levelData[y * LEVEL_W + x])) {
                    DrawRectangle(x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE, DARKGRAY);
                }
            }
        }

        // Draw player
        DrawTexturePro(
            pug,
            (Rectangle){0, 0, pug.width, pug.height},
            (Rectangle){
                player.x + player.width / 2 - pug.width / 2,
                player.y + player.height / 2 - pug.height / 2,
                pug.width, pug.height},
            (Vector2){0, 0}, 0.0f, WHITE);

        // Draw red dots
        for (int i = 0; i < MAX_DOTS; i++) {
            if (dots[i].alive) {
                DrawCircle(
                    (int)(dots[i].rect.x + dots[i].rect.width / 2),
                    (int)(dots[i].rect.y + dots[i].rect.height / 2),
                    dots[i].rect.width / 2,
                    (Color){139, 69, 19, 255}); // brown
            }
        }

        DrawText(TextFormat("Klopsiki zjedzone: %d", score), 2, 2, 24, RED);
        EndDrawing();
    }

    UnloadTexture(pug);
    CloseWindow();
    return 0;
}
