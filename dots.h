#ifndef DOTS_H
#define DOTS_H

#include "raylib.h"
#include "config.h"

typedef struct {
    Rectangle rect;
    int       alive;
    Vector2   last_pos;
    int       stuck_frames;
} RedDot;

void SpawnDots (RedDot dots[MAX_DOTS], const Rectangle *player);
void UpdateDots(RedDot dots[MAX_DOTS], const Rectangle *player);
void DrawDots  (const RedDot dots[MAX_DOTS]);
int  CheckDotCollision(RedDot dots[MAX_DOTS], const Rectangle *player); /* returns #eaten */

#endif /* DOTS_H */
