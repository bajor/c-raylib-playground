#include "raylib.h"

#define SCREEN_W 800
#define SCREEN_H 450
#define BALL_R 8

typedef struct {
  Rectangle r;
  int speed;
} Paddle;

int main(void) {
  InitWindow(SCREEN_W, SCREEN_H, "Pong – raylib");
  SetTargetFPS(60);

  Paddle left = {(Rectangle){20, SCREEN_H / 2 - 60, 10, 120}, 6};
  Paddle right = {(Rectangle){SCREEN_W - 30, SCREEN_H / 2 - 60, 10, 120}, 6};

  Vector2 ballPos = {SCREEN_W / 2.0f, SCREEN_H / 2.0f};
  Vector2 ballVel = {4, 4};

  int scoreL = 0, scoreR = 0;

  while (!WindowShouldClose()) {
    /* ---- input ---- */
    if (IsKeyDown(KEY_W) && left.r.y > 0)
      left.r.y -= left.speed;
    if (IsKeyDown(KEY_S) && left.r.y + left.r.height < SCREEN_H)
      left.r.y += left.speed;
    if (IsKeyDown(KEY_UP) && right.r.y > 0)
      right.r.y -= right.speed;
    if (IsKeyDown(KEY_DOWN) && right.r.y + right.r.height < SCREEN_H)
      right.r.y += right.speed;

    /* ---- physics ---- */
    ballPos.x += ballVel.x;
    ballPos.y += ballVel.y;

    if (ballPos.y <= BALL_R || ballPos.y >= SCREEN_H - BALL_R)
      ballVel.y *= -1;

    if (CheckCollisionCircleRec(ballPos, BALL_R, left.r) && ballVel.x < 0)
      ballVel.x *= -1.05f;
    if (CheckCollisionCircleRec(ballPos, BALL_R, right.r) && ballVel.x > 0)
      ballVel.x *= -1.05f;

    if (ballPos.x < 0) {
      scoreR++;
      ballPos = (Vector2){SCREEN_W / 2, SCREEN_H / 2};
      ballVel = (Vector2){4, 4};
    }
    if (ballPos.x > SCREEN_W) {
      scoreL++;
      ballPos = (Vector2){SCREEN_W / 2, SCREEN_H / 2};
      ballVel = (Vector2){-4, -4};
    }

    /* ---- draw ---- */
    BeginDrawing();
    ClearBackground(BLACK);

    DrawRectangleRec(left.r, WHITE);
    DrawRectangleRec(right.r, WHITE);
    DrawCircleV(ballPos, BALL_R, RAYWHITE);
    DrawLine(SCREEN_W / 2, 0, SCREEN_W / 2, SCREEN_H, GRAY);

    DrawText(TextFormat("%d", scoreL), SCREEN_W / 4 - 10, 20, 40, GRAY);
    DrawText(TextFormat("%d", scoreR), 3 * SCREEN_W / 4 - 10, 20, 40, GRAY);

    EndDrawing();
  }
  CloseWindow();
  return 0;
}
