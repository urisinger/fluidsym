#include "grid.h"
#include <math.h>
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) < (b)) ? (a) : (b))

Grid grid_new_uninit(size_t size_x, size_t size_y) {
  Grid grid = {.size_x = size_x, .size_y = size_y};
  grid.v = malloc(sizeof(float) * size_x * size_y);
  grid.u = malloc(sizeof(float) * size_x * size_y);
  grid.new_v = malloc(sizeof(float) * size_x * size_y);
  grid.new_u = malloc(sizeof(float) * size_x * size_y);
  grid.p = malloc(sizeof(float) * (size_x - 1) * (size_y - 1));

  return grid;
}

Grid grid_new_uniform(size_t size_x, size_t size_y, float value,
                      float density) {
  Grid grid = grid_new_uninit(size_x, size_y);

  grid.density = density;
  grid.spacing = 1.0 / sqrtf(size_x * size_y);

  for (int i = 0; i < size_x * size_y; i++) {
    grid.v[i] = value;
    grid.u[i] = value;
    if (i < (size_x - 1) * (size_y - 1) + 1) {
      grid.p[i] = 0.0;
    }
  }

  return grid;
}

Grid grid_new_random(size_t size_x, size_t size_y, float min, float max,
                     float density) {
  Grid grid = grid_new_uninit(size_x, size_y);

  grid.density = density;
  grid.spacing = 1.0 / sqrtf(size_x * size_y);

  for (int i = 0; i < size_x * size_y; i++) {
    grid.v[i] = (float)rand() / (float)(RAND_MAX) * (min - max) + min;
    grid.u[i] = (float)rand() / (float)(RAND_MAX) * (min - max) + min;
    if (i < (size_x - 1) * (size_y - 1)) {
      grid.p[i] = 0.0;
    }
  }

  return grid;
}

float get_s(const Grid *grid, int i, int j) {
  if (i >= 0 && j >= 0 && i < grid->size_y - 1 && j < grid->size_x - 1) {
    return 1.0;
  }
  return 0.0;
}

#define GRAVITY -9.81

void add_gravity(Grid *grid, float dt) {
  for (int i = 1; i < grid->size_y - 1; i++) {
    for (int j = 1; j < grid->size_x - 1; j++) {
      grid->v[i * grid->size_x + j] += dt * GRAVITY;
    }
  }
}

void make_incompressible(Grid *grid, float dt) {
  float cp = (grid->density * grid->spacing) / dt;
  for (int i = 0; i < grid->size_y - 1; i++) {
    for (int j = 0; j < grid->size_x - 1; j++) {
      float s = get_s(grid, i + 1, j) + get_s(grid, i - 1, j) +
                get_s(grid, i, j + 1) + get_s(grid, i, j - 1);

      float d =
          
          1.8*(grid->u[i * grid->size_x + j + 1] - grid->u[i * grid->size_x + j] +
           grid->v[(i + 1) * grid->size_x + j] - grid->v[i * grid->size_x + j]);

      grid->u[i * grid->size_x + j] += (get_s(grid, i, j - 1) * d) / s;
      grid->u[i * grid->size_x + j + 1] -= (get_s(grid, i, j + 1) * d) / s;

      grid->v[i * grid->size_x + j] += (get_s(grid, i - 1, j) * d) / s;
      grid->v[(i + 1) * grid->size_x + j] -= (get_s(grid, i + 1, j) * d) / s;

      grid->p[i * (grid->size_x - 1) + j] += (d / s) * cp;
    }
  }
}

float avg_v(Grid *grid, int i, int j) {
  return (grid->v[i * grid->size_x + j] + grid->v[(i + 1) * grid->size_x + j] +
          grid->v[(i)*grid->size_x + j - 1] +
          grid->v[(i + 1) * grid->size_x + j - 1]) /
         4;
}

float avg_u(Grid *grid, int i, int j) {
  return (grid->u[i * grid->size_x + j] + grid->u[(i - 1) * grid->size_x + j] +
          grid->v[(i)*grid->size_x + j + 1] +
          grid->v[(i - 1) * grid->size_x + j + 1]) /
         4;
}

void advent_velocity(Grid *grid, float dt) {
  memcpy(grid->new_u, grid->u, sizeof(float) * grid->size_x * grid->size_y);
  memcpy(grid->new_v, grid->v, sizeof(float) * grid->size_x * grid->size_y);

  for (int i = 0; i < grid->size_x; i++) {
    for (int j = 0; j < grid->size_y; j++) {

      if (get_s(grid, i, j) != 0.0 && get_s(grid, i + 1, j) != 0.0 &&
          get_s(grid, i - 1, j) != 0.0) {
        float x =
            max(0, min((float)grid->size_x, (float)j - avg_u(grid, i, j) * dt));
        float y = max(0, min((float)grid->size_y,
                             (float)i - grid->v[i * grid->size_x + j] * dt));
        float dx = x - floorf(x);
        float dy = y - floorf(y);

        int x0 = (int)floorf(x);
        int y0 = (int)floorf(y);
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        x0 = max(min(x0, grid->size_x - 1), 0);
        x1 = max(min(x1, grid->size_x - 1), 0);
        y0 = max(min(y0, grid->size_y - 1), 0);
        y1 = max(min(y1, grid->size_y - 1), 0);

        float v00 = grid->v[y0 * grid->size_x + x0];
        float v01 = grid->v[y1 * grid->size_x + x0];
        float v10 = grid->v[y0 * grid->size_x + x1];
        float v11 = grid->v[y1 * grid->size_x + x1];

        float v = (1 - dx) * ((1 - dy) * v00 + dy * v01) +
                  dx * ((1 - dy) * v10 + dy * v11);

        grid->new_v[i * grid->size_x + j] = v;
      }

      if (get_s(grid, i, j) != 0.0 && get_s(grid, i, j + 1) != 0.0 &&
          get_s(grid, i, j - 1) != 0.0) {
        float x =
            max(0, min((float)grid->size_x, (float)j - grid->v[i * grid->size_x + j] * dt));
        float y = max(0, min((float)grid->size_y,
                             (float)i -  avg_v(grid, i, j)* dt));
        float dx = x - floorf(x);
        float dy = y - floorf(y);

        int x0 = (int)floorf(x);
        int y0 = (int)floorf(y);
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        x0 = max(min(x0, grid->size_x - 1), 0);
        x1 = max(min(x1, grid->size_x - 1), 0);
        y0 = max(min(y0, grid->size_y - 1), 0);
        y1 = max(min(y1, grid->size_y - 1), 0);

        float v00 = grid->u[y0 * grid->size_x + x0];
        float v01 = grid->u[y1 * grid->size_x + x0];
        float v10 = grid->u[y0 * grid->size_x + x1];
        float v11 = grid->u[y1 * grid->size_x + x1];

        float v = (1 - dx) * ((1 - dy) * v00 + dy * v01) +
                  dx * ((1 - dy) * v10 + dy * v11);

        grid->new_u[i * grid->size_x + j] = v;
      }
    }
  }
  memcpy(grid->u, grid->new_u, sizeof(float) * grid->size_x * grid->size_y);
  memcpy(grid->v, grid->new_v, sizeof(float) * grid->size_x * grid->size_y);
}

#define NUM_ITERATIONS 100

void advance_grid(Grid *grid, float dt) {
  add_gravity(grid, dt);

  for (int n = 0; n < NUM_ITERATIONS; n++) {
    make_incompressible(grid, dt);
  }
  advent_velocity(grid, dt);
}

Color getSquareColor(float val, float min, float max) {

  // Clamp val to the range [minVal, maxVal - 0.0001]
  if (val < min)
    val = min;
  if (val > max - 0.0001f)
    val = max - 0.0001f;

  float d = max - min;
  val = (d == 0.0f) ? 0.5f : (val - min) / d;
  float m = 0.25f;
  int num = (int)(val / m);
  float s = (val - num * m) / m;
  float r = 0.0f, g = 0.0f, b = 0.0f;

  switch (num) {
  case 0:
    r = 0.0f;
    g = s;
    b = 1.0f;
    break;
  case 1:
    r = 0.0f;
    g = 1.0f;
    b = 1.0f - s;
    break;
  case 2:
    r = s;
    g = 1.0f;
    b = 0.0f;
    break;
  case 3:
    r = 1.0f;
    g = 1.0f - s;
    b = 0.0f;
    break;
  }
  Vector4 color = {r, g, b, 1.0};
  return ColorFromNormalized(color);
}

void render_grid(const Grid *grid, int screen_width, int screen_height) {

  float minP = grid->p[0];
  float maxP = grid->p[0];

  for (int i = 0; i < (grid->size_y - 1) * (grid->size_x - 1); i++) {
    float val = grid->p[i];
    if (val < minP) {
      minP = val;
    }
    if (val > maxP) {
      maxP = val;
    }
  }

  Vector2 size = {.x = (float)screen_width / (grid->size_x - 1),
                  .y = (float)screen_height / (grid->size_y - 1)};
  for (int i = 0; i < grid->size_y - 1; i++) {
    for (int j = 0; j < grid->size_x - 1; j++) {
      int index = i * (grid->size_x - 1) + j;
      Vector2 corner = {.x = size.x * j, .y = size.y * i};

      DrawRectangleV(corner, size,
                     getSquareColor(grid->p[index], minP, maxP + 10.0));
    }
  }
}
