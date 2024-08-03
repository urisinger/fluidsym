#include "fluid.h"

#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <string.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

Fluid grid_new_uninit(size_t size_x, size_t size_y) {
  Fluid fluid = {.size_x = size_x, .size_y = size_y};

  fluid.s = malloc(sizeof(float) * (size_x - 1) * (size_y - 1));
  fluid.y_velo = malloc(sizeof(float) * size_x * size_y);
  fluid.x_velo = malloc(sizeof(float) * size_x * size_y);

  fluid.pressure = malloc(sizeof(float) * (size_x - 1) * (size_y - 1));

  fluid.old_smoke = malloc(sizeof(float) * (size_x - 1) * (size_y - 1));
  fluid.smoke = malloc(sizeof(float) * (size_x - 1) * (size_y - 1));

  fluid.new_v = malloc(sizeof(float) * size_x * size_y);
  fluid.new_u = malloc(sizeof(float) * size_x * size_y);
  return fluid;
}

Fluid fluid_new(size_t size_x, size_t size_y, float overrelax,  float height) {
  Fluid fluid = grid_new_uninit(size_x, size_y);

  fluid.density = 1.0;
  fluid.spacing = height / size_y;
  fluid.overrelax = overrelax;

  for (int i = 0; i < size_x * size_y; i++) {
    fluid.y_velo[i] = 0.0;
    fluid.x_velo[i] = 0.0;
    if (i < (size_x - 1) * (size_y - 1) + 1) {
      fluid.s[i] = 1.0;
      fluid.smoke[i] = 1.0;
      fluid.pressure[i] = 0.0;
    }
  }

  return fluid;
}

void fluid_free(Fluid *f) {
  free(f->s);
  free(f->y_velo);
  free(f->x_velo);
  free(f->pressure);
  free(f->old_smoke);
  free(f->smoke);
  free(f->new_v);
  free(f->new_u);
}

float get_s(const Fluid *f, int i, int j) {
  if (i >= 0 && j >= 0 && i < f->size_y - 1 && j < f->size_x - 1) {
    return f->s[i * (f->size_x - 1) + j];
  }
  return 0.0;
}

#define GRAVITY 9.81

void add_gravity(Fluid *f, float dt) {
  for (int i = 1; i < f->size_y - 1; i++) {
    for (int j = 1; j < f->size_x - 1; j++) {
      if (get_s(f, i, j) != 0.0 && get_s(f, i - 1, j) != 0.0 &&
          get_s(f, i + 1, j) != 0.0) {
        f->y_velo[i * f->size_x + j] += dt * GRAVITY;
      }
    }
  }
}

void make_incompressible(Fluid *f, float dt) {
  float cp = (f->density * f->spacing) / dt;

  for (int i = 0; i < f->size_y - 1; i++) {
    for (int j = 0; j < f->size_x - 1; j++) {
      int n = f->size_x;
      if (get_s(f, i, j) == 0.0) {
        continue;
      }

      float s1 = get_s(f, i + 1, j);
      float s2 = get_s(f, i - 1, j);
      float s3 = get_s(f, i, j + 1);
      float s4 = get_s(f, i, j - 1);

      float s = s1 + s2 + s3 + s4;

      float vx = f->x_velo[i * n + j + 1] - f->x_velo[i * n + j];
      float vy = f->y_velo[(i + 1) * n + j] - f->y_velo[i * n + j];

      float d = vx + vy;

      float p = d / s;
      p *= f->overrelax;

      f->y_velo[(i + 1) * n + j] -= p * s1;
      f->y_velo[i * n + j] += p * s2;

      f->x_velo[i * n + j + 1] -= p * s3;
      f->x_velo[i * n + j] += p * s4;

      f->pressure[i * (n - 1) + j] -= p * cp;
    }
  }
}

float avg_v(Fluid *f, int i, int j) {
  int n = f->size_x;

  return (f->y_velo[i * n + j] + f->y_velo[(i + 1) * n + j] +
          f->y_velo[(i)*n + j - 1] + f->y_velo[(i + 1) * n + j - 1]) /
         4;
}

float avg_u(Fluid *f, int i, int j) {
  int n = f->size_x;
  return (f->x_velo[i * n + j] + f->x_velo[(i - 1) * n + j] +
          f->y_velo[(i)*n + j + 1] + f->y_velo[(i - 1) * n + j + 1]) /
         4;
}

float sample_grid(Fluid* f, float *grid, int n, float x, float y) {
  float dx = x - floorf(x);
  float dy = y - floorf(y);

  int x0 = (int)floorf(x);
  int y0 = (int)floorf(y);
  int x1 = x0 + 1;
  int y1 = y0 + 1;

  x0 = max(min(x0, n - 1), 0);
  x1 = max(min(x1, n - 1), 0);
  y0 = max(min(y0, n - 1), 0);
  y1 = max(min(y1, n - 1), 0);

  float v00 = grid[y0 * n + x0] * get_s(f,y0, x0);
  float v01 = grid[y1 * n + x0] * get_s(f,y1, x0);
  float v10 = grid[y0 * n + x1] * get_s(f,y0, x1);
  float v11 = grid[y1 * n + x1] * get_s(f,y1, x1);

  float v =
      (1 - dx) * ((1 - dy) * v00 + dy * v01) + dx * ((1 - dy) * v10 + dy * v11);

  return v;
}

void advent_velocity(Fluid *f, float dt) {
  memcpy(f->new_u, f->x_velo, sizeof(float) * f->size_x * f->size_y);
  memcpy(f->new_v, f->y_velo, sizeof(float) * f->size_x * f->size_y);

  for (int i = 0; i < f->size_y; i++) {
    for (int j = 0; j < f->size_x; j++) {

      if (get_s(f, i, j) != 0.0 && get_s(f, i - 1, j)) {
        float x = (float)j - avg_u(f, i, j) * dt / f->spacing;
        float y = (float)i - f->y_velo[i * f->size_x + j] * dt / f->spacing;

        f->new_v[i * f->size_x + j] = sample_grid(f, f->y_velo, f->size_x, x, y);
      }

      if (get_s(f, i, j) != 0.0 && get_s(f, i, j - 1) != 0.0) {
        float x = (float)j - f->x_velo[i * f->size_x + j] * dt / f->spacing;
        float y = (float)i - avg_v(f, i, j) * dt / f->spacing;

        f->new_u[i * f->size_x + j] = sample_grid(f, f->x_velo, f->size_x, x, y);
      }
    }
  }
  memcpy(f->x_velo, f->new_u, sizeof(float) * f->size_x * f->size_y);
  memcpy(f->y_velo, f->new_v, sizeof(float) * f->size_x * f->size_y);
}

void advent_smoke(Fluid *f, float dt) {
  memcpy(f->old_smoke, f->smoke,
         sizeof(float) * (f->size_x - 1) * (f->size_y - 1));
  float h = dt / f->spacing;
  for (int i = 0; i < f->size_y - 1; i++) {
    for (int j = 0; j < f->size_x - 1; j++) {
      if (get_s(f, i, j) != 0.0) {
        float dx =
            f->x_velo[i * f->size_x + j] + f->x_velo[i * f->size_x + j + 1];
        float x = (float)j - dx * h * 0.5;

        float dy =
            f->y_velo[i * f->size_x + j] + f->y_velo[(i + 1) * f->size_x + j];
        float y = (float)i - dy * h * 0.5;

        f->smoke[i * (f->size_x - 1) + j] =
            sample_grid(f, f->old_smoke, f->size_x - 1, x, y);
      }
    }
  }
}

#define NUM_ITERATIONS 75

void advance_grid(Fluid *f, float dt) {
  memset(f->pressure, 0.0, sizeof(float) * (f->size_x - 1) * (f->size_y - 1));
  for (int n = 0; n < NUM_ITERATIONS; n++) {
    make_incompressible(f, dt);
  }

  advent_velocity(f, dt);

  advent_smoke(f, dt);
  f->frame += 1.0;
}

void move_ball(Fluid *f, Vector2 last_pos, Vector2 next_pos, float r,
               float dt) {

  Vector2 v = Vector2Scale(Vector2Subtract(next_pos, last_pos),
                           f->spacing * f->size_y / (dt));

  for (int i = 0; i < f->size_y - 1; i++) {
    for (int j = 0; j < f->size_x - 1; j++) {
      f->s[i * (f->size_x - 1) + j] = 1.0;

      Vector2 d = {
          .x = (j + 0.5) * f->spacing - next_pos.x * f->size_x * f->spacing,
          .y = (i + 0.5) * f->spacing - next_pos.y * f->size_y * f->spacing};

      if (Vector2LengthSqr(d) < r * r) {
        f->s[i * (f->size_x - 1) + j] = 0.0;
        if (fabsf(v.x) > 0.1) {
          f->x_velo[i * f->size_x + j] = v.x;
          f->x_velo[(i + 1) * f->size_x + j] = v.x;
        }
        if (fabsf(v.y) > 0.1) {
          f->y_velo[i * f->size_x + j] = v.y;
          f->y_velo[i * f->size_x + j + 1] = v.y;
        }

        f->smoke[i * (f->size_x - 1) + j] = 0.5 + 0.5 * sinf(f->frame * 0.01);
      }
    }
  }
}
