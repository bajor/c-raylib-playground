#include "dots.h"
#include "level.h"
#include <stdlib.h>
#include <math.h>

static inline float dist(float x1,float y1,float x2,float y2)
{
    float dx = x2 - x1, dy = y2 - y1;
    return sqrtf(dx*dx + dy*dy);
}

void SpawnDots(RedDot dots[MAX_DOTS], const Rectangle *player)
{
    int spawned = 0;
    while (spawned < MAX_DOTS) {
        int gx = rand() % LEVEL_W;
        int gy = rand() % LEVEL_H;
        if (IsWall(levelData[gy*LEVEL_W + gx])) continue;

        float px = player->x + PLAYER_SIZE/2.0f;
        float py = player->y + PLAYER_SIZE/2.0f;
        float cx = gx*TILE_SIZE + TILE_SIZE/2.0f;
        float cy = gy*TILE_SIZE + TILE_SIZE/2.0f;
        if (dist(px,py,cx,cy) < TILE_SIZE*4) continue;      /* keep distance */

        dots[spawned] = (RedDot){
            .rect = {gx*TILE_SIZE + TILE_SIZE/4.0f,
                     gy*TILE_SIZE + TILE_SIZE/4.0f,
                     TILE_SIZE/2.0f, TILE_SIZE/2.0f},
            .alive = 1,
            .last_pos = {cx, cy},
            .stuck_frames = 0
        };
        ++spawned;
    }
}

void UpdateDots(RedDot dots[MAX_DOTS], const Rectangle *player)
{
    for (int i = 0; i < MAX_DOTS; ++i) {
        if (!dots[i].alive) continue;

        float moved = dist(dots[i].rect.x, dots[i].rect.y,
                           dots[i].last_pos.x, dots[i].last_pos.y);
        dots[i].stuck_frames = (moved < 1.0f) ? dots[i].stuck_frames + 1 : 0;
        dots[i].last_pos = (Vector2){dots[i].rect.x, dots[i].rect.y};

        float random_chance = (dots[i].stuck_frames > 10) ? 0.5f : 0.1f;
        float player_dist   = dist(dots[i].rect.x, dots[i].rect.y, player->x, player->y);
        bool  force_random  = (dots[i].stuck_frames > 10 && player_dist < 200.0f);
        bool  do_random     = force_random || ((float)rand()/RAND_MAX < random_chance);

        float best_score = -1e9f;
        Vector2 best_move = {0,0};
        Vector2 valid[8];
        int     valid_n = 0;

        for (int dx = -1; dx <= 1; ++dx)
            for (int dy = -1; dy <= 1; ++dy)
            {
                if (dx==0 && dy==0) continue;
                Rectangle test = dots[i].rect;
                test.x += dx*DOT_SPEED;
                test.y += dy*DOT_SPEED;
                if (CollidesWithLevel(test)) continue;

                valid[valid_n++] = (Vector2){dx*DOT_SPEED, dy*DOT_SPEED};

                float d_player = dist(test.x,test.y, player->x, player->y);
                float d_closest = 1e9f;
                for (int j = 0; j < MAX_DOTS; ++j)
                    if (i!=j && dots[j].alive) {
                        float d = dist(test.x,test.y, dots[j].rect.x, dots[j].rect.y);
                        if (d < d_closest) d_closest = d;
                    }
                float score = d_player + 0.7f*d_closest;
                if (score > best_score) {
                    best_score = score;
                    best_move  = (Vector2){dx*DOT_SPEED, dy*DOT_SPEED};
                }
            }

        if (do_random && valid_n > 0)
            best_move = valid[rand() % valid_n];

        dots[i].rect.x += best_move.x;
        dots[i].rect.y += best_move.y;
    }
}

void DrawDots(const RedDot dots[MAX_DOTS])
{
    for (int i = 0; i < MAX_DOTS; ++i)
        if (dots[i].alive)
            DrawCircle((int)(dots[i].rect.x + dots[i].rect.width /2.0f),
                       (int)(dots[i].rect.y + dots[i].rect.height/2.0f),
                       dots[i].rect.width / 2.0f,
                       (Color){139,69,19,255});
}

int CheckDotCollision(RedDot dots[MAX_DOTS], const Rectangle *player)
{
    int eaten = 0;
    for (int i = 0; i < MAX_DOTS; ++i)
        if (dots[i].alive && CheckCollisionRecs(*player, dots[i].rect)) {
            dots[i].alive = 0;
            ++eaten;
        }
    return eaten;
}
