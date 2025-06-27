#include "dots.h"
#include "level.h"
#include <math.h>
#include <stdlib.h>

static inline float dist(float x1, float y1, float x2, float y2) {
  float dx = x2 - x1, dy = y2 - y1;
  return sqrtf(dx * dx + dy * dy);
}

// Flood fill to count open area from (x, y) within radius (in px)
static int FloodFillOpenArea(float x, float y, float radius) {
  int tile_radius = (int)(radius / TILE_SIZE) + 1;
  int cx = (int)(x / TILE_SIZE);
  int cy = (int)(y / TILE_SIZE);
  bool visited[2 * tile_radius + 1][2 * tile_radius + 1];
  for (int i = 0; i < 2 * tile_radius + 1; ++i)
    for (int j = 0; j < 2 * tile_radius + 1; ++j)
      visited[i][j] = false;
  int open = 0;
  int queue[256][2];
  int qh = 0, qt = 0;
  queue[qh][0] = cx;
  queue[qh][1] = cy;
  ++qh;
  visited[tile_radius][tile_radius] = true;
  while (qt < qh) {
    int tx = queue[qt][0], ty = queue[qt][1];
    ++qt;
    float wx = tx * TILE_SIZE + TILE_SIZE / 2.0f;
    float wy = ty * TILE_SIZE + TILE_SIZE / 2.0f;
    if (dist(wx, wy, x, y) > radius)
      continue;
    if (tx < 0 || ty < 0 || tx >= LEVEL_W || ty >= LEVEL_H)
      continue;
    if (IsWall(levelData[ty * LEVEL_W + tx]))
      continue;
    ++open;
    for (int dx = -1; dx <= 1; ++dx)
      for (int dy = -1; dy <= 1; ++dy) {
        if (abs(dx) + abs(dy) != 1)
          continue; // 4 directions
        int nx = tx + dx, ny = ty + dy;
        int ix = nx - cx + tile_radius, iy = ny - cy + tile_radius;
        if (ix < 0 || iy < 0 || ix > 2 * tile_radius || iy > 2 * tile_radius)
          continue;
        if (!visited[ix][iy]) {
          visited[ix][iy] = true;
          queue[qh][0] = nx;
          queue[qh][1] = ny;
          ++qh;
        }
      }
  }
  return open;
}

// Multi-step lookahead: recursively simulate up to 'depth' moves ahead, return
// min open area found
static int SimulateOpenArea(float x, float y, int depth, float radius) {
  int open = FloodFillOpenArea(x, y, radius);
  if (depth <= 0)
    return open;
  int best = open;
  for (int dx = -1; dx <= 1; ++dx)
    for (int dy = -1; dy <= 1; ++dy) {
      if (dx == 0 && dy == 0)
        continue;
      float nx = x + dx * DOT_SPEED;
      float ny = y + dy * DOT_SPEED;
      Rectangle test = {nx - 0.5f, ny - 0.5f, 1, 1};
      if (CollidesWithLevel(test))
        continue;
      int look = SimulateOpenArea(nx, ny, depth - 1, radius);
      if (look < best)
        best = look;
    }
  return best;
}

void SpawnDots(RedDot dots[MAX_DOTS], const Rectangle *player) {
  int spawned = 0;
  while (spawned < MAX_DOTS) {
    int gx = rand() % LEVEL_W;
    int gy = rand() % LEVEL_H;
    if (IsWall(levelData[gy * LEVEL_W + gx]))
      continue;

    float px = player->x + PLAYER_SIZE / 2.0f;
    float py = player->y + PLAYER_SIZE / 2.0f;
    float cx = gx * TILE_SIZE + TILE_SIZE / 2.0f;
    float cy = gy * TILE_SIZE + TILE_SIZE / 2.0f;
    if (dist(px, py, cx, cy) < TILE_SIZE * 4)
      continue; /* keep distance */

    dots[spawned] = (RedDot){.rect = {gx * TILE_SIZE + TILE_SIZE / 4.0f,
                                      gy * TILE_SIZE + TILE_SIZE / 4.0f,
                                      TILE_SIZE / 2.0f, TILE_SIZE / 2.0f},
                             .alive = 1,
                             .last_pos = {cx, cy},
                             .stuck_frames = 0,
                             .corner_frames = 0,
                             .last_dir = {0, 0}};
    ++spawned;
  }
}

void UpdateDots(RedDot dots[MAX_DOTS], const Rectangle *player) {
  for (int i = 0; i < MAX_DOTS; ++i) {
    if (!dots[i].alive)
      continue;

    float moved = dist(dots[i].rect.x, dots[i].rect.y, dots[i].last_pos.x,
                       dots[i].last_pos.y);
    dots[i].stuck_frames = (moved < 1.0f) ? dots[i].stuck_frames + 1 : 0;
    dots[i].last_pos = (Vector2){dots[i].rect.x, dots[i].rect.y};

    float random_chance = (dots[i].stuck_frames > 10) ? 0.5f : 0.1f;
    float player_dist =
        dist(dots[i].rect.x, dots[i].rect.y, player->x, player->y);
    bool force_random = (dots[i].stuck_frames > 10 && player_dist < 200.0f);
    bool do_random = force_random || ((float)rand() / RAND_MAX < random_chance);

    float best_score = -1e9f;
    Vector2 best_move = {0, 0};
    Vector2 valid[8];
    int valid_n = 0;

    for (int dx = -1; dx <= 1; ++dx)
      for (int dy = -1; dy <= 1; ++dy) {
        if (dx == 0 && dy == 0)
          continue;
        if (dots[i].last_dir.x == -dx * DOT_SPEED &&
            dots[i].last_dir.y == -dy * DOT_SPEED)
          continue;
        Rectangle test = dots[i].rect;
        test.x += dx * DOT_SPEED;
        test.y += dy * DOT_SPEED;
        if (CollidesWithLevel(test))
          continue;

        valid[valid_n++] = (Vector2){dx * DOT_SPEED, dy * DOT_SPEED};

        float d_player = dist(test.x, test.y, player->x, player->y);
        int future_open = SimulateOpenArea(
            test.x + test.width / 2.0f, test.y + test.height / 2.0f, 2, 20.0f);
        float score = future_open + d_player * 0.1f;
        if (score > best_score) {
          best_score = score;
          best_move = (Vector2){dx * DOT_SPEED, dy * DOT_SPEED};
        }
      }

    if (do_random && valid_n > 0)
      best_move = valid[rand() % valid_n];

    if (valid_n == 0) {
      for (int dx = -1; dx <= 1; ++dx)
        for (int dy = -1; dy <= 1; ++dy) {
          if (dx == 0 && dy == 0)
            continue;
          Rectangle test = dots[i].rect;
          test.x += dx * DOT_SPEED;
          test.y += dy * DOT_SPEED;
          if (CollidesWithLevel(test))
            continue;
          float d_player = dist(test.x, test.y, player->x, player->y);
          int future_open =
              SimulateOpenArea(test.x + test.width / 2.0f,
                               test.y + test.height / 2.0f, 2, 20.0f);
          float score = future_open + d_player * 0.1f;
          if (score > best_score) {
            best_score = score;
            best_move = (Vector2){dx * DOT_SPEED, dy * DOT_SPEED};
          }
        }
    }

    if (best_move.x != 0 || best_move.y != 0) {
      dots[i].rect.x += best_move.x;
      dots[i].rect.y += best_move.y;
      dots[i].last_dir = best_move;
    }
  }
}

void DrawDots(const RedDot dots[MAX_DOTS]) {
  for (int i = 0; i < MAX_DOTS; ++i)
    if (dots[i].alive)
      DrawCircle((int)(dots[i].rect.x + dots[i].rect.width / 2.0f),
                 (int)(dots[i].rect.y + dots[i].rect.height / 2.0f),
                 dots[i].rect.width / 2.0f, (Color){139, 69, 19, 255});
}

int CheckDotCollision(RedDot dots[MAX_DOTS], const Rectangle *player) {
  int eaten = 0;
  for (int i = 0; i < MAX_DOTS; ++i)
    if (dots[i].alive && CheckCollisionRecs(*player, dots[i].rect)) {
      dots[i].alive = 0;
      ++eaten;
    }
  return eaten;
}
