#ifndef LEVEL_H_GUARD
#define LEVEL_H_GUARD

#include "config.h"
#include "raylib.h"
#include <stdbool.h>

extern Color levelData[LEVEL_W * LEVEL_H];

bool LoadLevel(const char *imagePath); /* PNG → tiles */
bool CollidesWithLevel(Rectangle r);   /* AABB vs. walls */
void DrawLevel(void);                  /* paint walls */
bool IsWall(Color c);                  /* utility */

#endif /* LEVEL_H_GUARD */
