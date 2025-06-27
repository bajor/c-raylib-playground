#include "raylib.h"
#include "config.h"
#include "level.h"
#include "dots.h"
#include <math.h>

int main(void)
{
    InitWindow(SCREEN_W, SCREEN_H, "Top-down Platformer – raylib");
    SetTargetFPS(60);

    Texture2D pug = LoadTexture("pug.png");

    if (!LoadLevel("level.png")) { CloseWindow(); return 1; }

    Rectangle player = {TILE_SIZE+4, TILE_SIZE+4, PLAYER_SIZE, PLAYER_SIZE};
    Vector2   vel    = {0,0};

    RedDot dots[MAX_DOTS] = {0};
    SpawnDots(dots, &player);

    int score = 0;

    while (!WindowShouldClose())
    {
        /* --- Input -------------------------------------------------------- */
        vel = (Vector2){0,0};
        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))    vel.y = -PLAYER_SPEED;
        if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))  vel.y =  PLAYER_SPEED;
        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))  vel.x = -PLAYER_SPEED;
        if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) vel.x =  PLAYER_SPEED;

        /* --- Player movement (full, X only, Y only) ----------------------- */
        Rectangle next = player;
        next.x += vel.x; next.y += vel.y;
        if      (!CollidesWithLevel(next))                     player = next;
        else {
            next = player; next.x += vel.x;
            if  (!CollidesWithLevel(next))                     player = next;
            else { next = player; next.y += vel.y;
                   if (!CollidesWithLevel(next))               player = next; }
        }

        /* --- Dots --------------------------------------------------------- */
        UpdateDots(dots, &player);
        score += CheckDotCollision(dots, &player);

        /* --- Rendering ---------------------------------------------------- */
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawLevel();
        DrawTexturePro(pug,
                       (Rectangle){0,0,pug.width,pug.height},
                       (Rectangle){player.x + player.width /2 - pug.width /2,
                                   player.y + player.height/2 - pug.height/2,
                                   pug.width, pug.height},
                       (Vector2){0,0}, 0.0f, WHITE);
        DrawDots(dots);
        DrawText(TextFormat("Klopsiki zjedzone: %d", score), 2, 2, 24, RED);

        EndDrawing();
    }

    UnloadTexture(pug);
    CloseWindow();
    return 0;
}
