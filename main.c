#include "config.h"
#include "dots.h"
#include "level.h"
#include "raylib.h"
#include <math.h>

int main(void) {
  InitWindow(SCREEN_W, SCREEN_H, "Top-down Platformer – raylib");
  SetTargetFPS(60);

  Texture2D pug = LoadTexture("pug.png");

  if (!LoadLevel("generated_maze.png")) {
    CloseWindow();
    return 1;
  }

  Rectangle player = {TILE_SIZE + 4, TILE_SIZE + 4, PLAYER_SIZE, PLAYER_SIZE};
  Vector2 vel = {0, 0};
  Vector2 last_dir = {1, 0}; // Default facing right

  RedDot dots[MAX_DOTS] = {0};
  SpawnDots(dots, &player);

  int score = 0;

  // Find a non-wall tile nearest the center for player start
  int center_x = LEVEL_W / 2;
  int center_y = LEVEL_H / 2;
  int best_dist = LEVEL_W * LEVEL_W + LEVEL_H * LEVEL_H;
  int best_x = center_x, best_y = center_y;
  for (int y = 0; y < LEVEL_H; ++y) {
    for (int x = 0; x < LEVEL_W; ++x) {
      if (!IsWall(levelData[y * LEVEL_W + x])) {
        int dx = x - center_x;
        int dy = y - center_y;
        int dist = dx * dx + dy * dy;
        if (dist < best_dist) {
          best_dist = dist;
          best_x = x;
          best_y = y;
        }
      }
    }
  }
  player.x = best_x * TILE_SIZE + (TILE_SIZE - player.width) / 2.0f;
  player.y = best_y * TILE_SIZE + (TILE_SIZE - player.height) / 2.0f;

  // Camera setup
  Camera2D camera = {0};
  camera.target = (Vector2){player.x + player.width / 2.0f,
                            player.y + player.height / 2.0f};
  camera.offset = (Vector2){SCREEN_W / 2.0f, SCREEN_H / 2.0f};
  camera.zoom = 1.0f;
  camera.rotation = 0.0f;

  while (!WindowShouldClose()) {
    /* --- Input -------------------------------------------------------- */
    vel = (Vector2){0, 0};
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
      vel.y = -PLAYER_SPEED;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
      vel.y = PLAYER_SPEED;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
      vel.x = -PLAYER_SPEED;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
      vel.x = PLAYER_SPEED;
    if (vel.x != 0 || vel.y != 0) {
      last_dir = vel;
    }

    /* --- Player movement (full, X only, Y only) ----------------------- */
    Rectangle next = player;
    next.x += vel.x;
    next.y += vel.y;
    if (!CollidesWithLevel(next))
      player = next;
    else {
      next = player;
      next.x += vel.x;
      if (!CollidesWithLevel(next))
        player = next;
      else {
        next = player;
        next.y += vel.y;
        if (!CollidesWithLevel(next))
          player = next;
      }
    }

    /* --- Update camera to follow player -------------------------------- */
    camera.target = (Vector2){player.x + player.width / 2.0f,
                              player.y + player.height / 2.0f};

    /* --- Dots --------------------------------------------------------- */
    UpdateDots(dots, &player);
    score += CheckDotCollision(dots, &player);

    /* --- Rendering ---------------------------------------------------- */
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode2D(camera);

    float angle = atan2f(last_dir.y, last_dir.x) * (180.0f / PI) - 90.0f;
    DrawLevel();
    DrawTexturePro(pug, (Rectangle){0, 0, pug.width, pug.height},
                   (Rectangle){player.x + player.width / 2,
                               player.y + player.height / 2, pug.width,
                               pug.height},
                   (Vector2){pug.width / 2, pug.height / 2}, angle, WHITE);
    DrawDots(dots);

    EndMode2D();

    DrawText(TextFormat("Klopsiki zjedzone: %d", score), 2, 2, 24, RED);
    DrawText(TextFormat("Player: (%.1f, %.1f)", player.x, player.y), 2, 30, 20,
             BLUE);
    DrawText(TextFormat("Camera Target: (%.1f, %.1f)", camera.target.x,
                        camera.target.y),
             2, 55, 20, GREEN);
    DrawText(TextFormat("Screen Center: (%d, %d)", SCREEN_W / 2, SCREEN_H / 2),
             2, 80, 20, RED);

    EndDrawing();
  }

  UnloadTexture(pug);
  CloseWindow();
  return 0;
}
