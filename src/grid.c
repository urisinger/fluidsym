#include "grid.h"
#include <math.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

Grid grid_new_uninit(size_t size_x, size_t size_y) {
  Grid grid = {.size_x = size_x, .size_y = size_y};
  grid.y_velo = malloc(sizeof(float) * size_x * size_y);
  grid.x_velo = malloc(sizeof(float) * size_x * size_y);
  grid.new_v = malloc(sizeof(float) * size_x * size_y);
  grid.new_u = malloc(sizeof(float) * size_x * size_y);
  grid.pressure = malloc(sizeof(float) * (size_x - 1) * (size_y - 1));
  grid.s = malloc(sizeof(float) * (size_x - 1) * (size_y - 1));

  grid.old_smoke = malloc(sizeof(float) * (size_x - 1) * (size_y - 1));
  grid.smoke = malloc(sizeof(float) * (size_x - 1) * (size_y - 1));

  return grid;
}

Grid grid_new_uniform(size_t size_x, size_t size_y, float value,
                      float density, float height) {
  Grid grid = grid_new_uninit(size_x, size_y);

  grid.density = density;
  grid.spacing = height / size_y;

  for (int i = 0; i < size_x * size_y; i++) {
    grid.y_velo[i] = value;
    grid.x_velo[i] = value;
    if (i < (size_x - 1) * (size_y - 1) + 1) {
      grid.s[i] = 1.0;
      grid.smoke[i] = 1.0;
      grid.pressure[i] = 0.0;
    }
  }

  return grid;
}

Grid grid_new_random(size_t size_x, size_t size_y, float min, float max,
                     float density, float height) {
  Grid grid = grid_new_uninit(size_x, size_y);

  grid.density = density;
  grid.spacing = height / size_y;

  for (int i = 0; i < size_x * size_y; i++) {
    grid.y_velo[i] = (float)rand() / (float)(RAND_MAX) * (min - max) + min;
    grid.x_velo[i] = (float)rand() / (float)(RAND_MAX) * (min - max) + min;
    if (i < (size_x - 1) * (size_y - 1)) {
      grid.pressure[i] = 0.0;
    }
  }

  return grid;
}

float get_s(const Grid *grid, int i, int j) {
  if (i >= 0 && j >= 0 && i < grid->size_y - 1 && j < grid->size_x - 1) {
    return grid->s[i * (grid->size_x - 1) + j];
  }
  return 0.0;
}

#define GRAVITY 9.81

void add_gravity(Grid *grid, float dt) {
  for (int i = 1; i < grid->size_y - 1; i++) {
    for (int j = 1; j < grid->size_x - 1; j++) {
      if (get_s(grid, i, j) != 0.0 && get_s(grid,i - 1, j) != 0.0 && get_s(grid,i + 1, j) != 0.0){
      grid->y_velo[i * grid->size_x + j] += dt * GRAVITY;
      }
    }
  }
}

void make_incompressible(Grid *grid, float dt) {
  float cp = (grid->density * grid->spacing) / dt;
  for (int i = 0; i < grid->size_y - 1; i++) {
    for (int j = 0; j < grid->size_x - 1; j++) {

      float s = get_s(grid, i + 1, j) + get_s(grid, i - 1, j) +
                get_s(grid, i, j + 1) + get_s(grid, i, j - 1);

      if (s == 0.0){
        continue;
      }
      float d =
          1.7*(grid->x_velo[i * grid->size_x + j + 1] - grid->x_velo[i * grid->size_x + j] +
           grid->y_velo[(i + 1) * grid->size_x + j] - grid->y_velo[i * grid->size_x + j]);

      float p = d / s;

      grid->x_velo[i * grid->size_x + j] += get_s(grid, i, j - 1) * p;
      grid->x_velo[i * grid->size_x + j + 1] -= p * get_s(grid, i, j + 1) ;

      grid->y_velo[i * grid->size_x + j] += p * get_s(grid, i - 1, j);
      grid->y_velo[(i + 1) * grid->size_x + j] -= p * get_s(grid, i + 1, j);

      grid->pressure[i * (grid->size_x - 1) + j] -= p * cp;
    }
  }
}

float avg_v(Grid *grid, int i, int j) {
  return (grid->y_velo[i * grid->size_x + j] + grid->y_velo[(i + 1) * grid->size_x + j] +
          grid->y_velo[(i)*grid->size_x + j - 1] +
          grid->y_velo[(i + 1) * grid->size_x + j - 1]) /
         4;
}

float avg_u(Grid *grid, int i, int j) {
  return (grid->x_velo[i * grid->size_x + j] + grid->x_velo[(i - 1) * grid->size_x + j] +
          grid->y_velo[(i)*grid->size_x + j + 1] +
          grid->y_velo[(i - 1) * grid->size_x + j + 1]) /
         4;
}

void advent_velocity(Grid *grid, float dt) {
  memcpy(grid->new_u, grid->x_velo, sizeof(float) * grid->size_x * grid->size_y);
  memcpy(grid->new_v, grid->y_velo, sizeof(float) * grid->size_x * grid->size_y);

  for (int i = 0; i < grid->size_x; i++) {
    for (int j = 0; j < grid->size_y; j++) {

      if (get_s(grid, i, j) != 0.0 && get_s(grid, i - 1, j) ) {
        float x =
            max(0, min((float)grid->size_x, (float)j - avg_u(grid, i, j) * dt/ grid->spacing));
        float y = max(0, min((float)grid->size_y,
                             (float)i - grid->y_velo[i * grid->size_x + j] * dt / grid->spacing));
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

        float v00 = grid->y_velo[y0 * grid->size_x + x0];
        float v01 = grid->y_velo[y1 * grid->size_x + x0];
        float v10 = grid->y_velo[y0 * grid->size_x + x1];
        float v11 = grid->y_velo[y1 * grid->size_x + x1];

        float v = (1 - dx) * ((1 - dy) * v00 + dy * v01) +
                  dx * ((1 - dy) * v10 + dy * v11);

        grid->new_v[i * grid->size_x + j] = v;
      }

      if (get_s(grid, i, j) != 0.0 && get_s(grid, i, j - 1) != 0.0) {
        float x =
            max(0, min((float)grid->size_x, (float)j - grid->x_velo[i * grid->size_x + j] * dt / grid->spacing));
        float y = max(0, min((float)grid->size_y,
                             (float)i -  avg_v(grid, i, j) * dt / grid->spacing));
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

        float v00 = grid->x_velo[y0 * grid->size_x + x0];
        float v01 = grid->x_velo[y1 * grid->size_x + x0];
        float v10 = grid->x_velo[y0 * grid->size_x + x1];
        float v11 = grid->x_velo[y1 * grid->size_x + x1];

        float v = (1 - dx) * ((1 - dy) * v00 + dy * v01) +
                  dx * ((1 - dy) * v10 + dy * v11);

        grid->new_u[i * grid->size_x + j] = v;
      }
    }
  }
  memcpy(grid->x_velo, grid->new_u, sizeof(float) * grid->size_x * grid->size_y);
  memcpy(grid->y_velo, grid->new_v, sizeof(float) * grid->size_x * grid->size_y);
}


void advent_smoke(Grid *grid, float dt) {

  memcpy(grid->old_smoke, grid->smoke,sizeof(float) * (grid->size_x - 1) * (grid->size_y - 1));
  for (int i = 0; i < grid->size_x - 1; i++) {
    for (int j = 0; j < grid->size_y - 1; j++) {
      if (get_s(grid, i, j) != 0.0) {
        float x =
            max(0, min((float)grid->size_x, (float)j - ((grid->x_velo[i * grid->size_x + j] + grid->x_velo[i * grid->size_x + j + 1])) * 0.5 * dt/ grid->spacing));
        float y = max(0, min((float)grid->size_y,
                             (float)i -  ((grid->y_velo[i * grid->size_x + j] + grid->y_velo[(i+1) * grid->size_x + j])) * 0.5 * dt / grid->spacing));
        float dx = x - floorf(x);
        float dy = y - floorf(y);

        int x0 = (int)floorf(x);
        int y0 = (int)floorf(y);
        int x1 = x0 + 1;
        int y1 = y0 + 1;

        x0 = max(min(x0, grid->size_x - 2), 0);
        x1 = max(min(x1, grid->size_x - 2), 0);
        y0 = max(min(y0, grid->size_y - 2), 0);
        y1 = max(min(y1, grid->size_y - 2), 0);

        float s00 = grid->old_smoke[y0 * (grid->size_x - 1)+ x0] ;
        float s01 = grid->old_smoke[y1 *(grid->size_x - 1) + x0] ;
        float s10 = grid->old_smoke[y0 * (grid->size_x - 1) + x1] ;
        float s11 = grid->old_smoke[y1 * (grid->size_x - 1) + x1];



        float v = (1 - dx) * ((1 - dy) * s00 + dy * s01) +
                  dx * ((1 - dy) * s10 + dy * s11);

        grid->smoke[i * (grid->size_x-1)+ j] = v;

      }

    }
  }


}

#define NUM_ITERATIONS 50

void advance_grid(Grid *grid, float dt) {

 //add_gravity(grid, dt);

  memset(grid->pressure, 0.0, sizeof(float) * (grid->size_x - 1) * (grid->size_y - 1));
  for (int n = 0; n < NUM_ITERATIONS; n++) {
    make_incompressible(grid, dt);
  }

  advent_velocity(grid, dt);

  advent_smoke(grid,dt);
  grid ->frame += 1.0;
}

void move_ball(Grid *grid, Vector2 last_pos, Vector2 next_pos, float r, float dt){

  Vector2 v = Vector2Scale(Vector2Subtract(next_pos, last_pos),  grid->spacing * grid->size_y / (dt));  

  for(int i = 0; i < grid->size_x - 1; i++){
    for (int j = 0; j < grid->size_y - 1; j++){
      grid->s[i * (grid->size_x - 1)+ j] = 1.0;

      Vector2 d = {.x = (j + 0.5) * grid->spacing - next_pos.x * grid->size_x * grid->spacing, .y = (i + 0.5) * grid->spacing - next_pos.y * grid->size_y * grid->spacing};

      if (Vector2LengthSqr(d) < r * r){
        grid->s[i * (grid->size_x - 1)+ j] = 0.0;
        if (fabsf(v.x) > 0.1){
					grid->x_velo[i*grid->size_x + j] = v.x;
					grid->x_velo[(i+1)*grid->size_x + j] = v.x;
        }
        if (fabsf(v.y) > 0.1){
					grid->y_velo[i*grid->size_x + j] = v.y;
					grid->y_velo[i*grid->size_x + j+1] = v.y;
        }

        grid->smoke[i * (grid->size_x - 1)+ j] = 0.5 + 0.5 * sinf(grid->frame * 0.001);
      } 
    }
  }
}
