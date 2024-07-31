#include <stddef.h>

typedef struct Grid{
  size_t size_x;
  size_t size_y;

  float* new_v;
  float* new_u;

  float* v;
  float* u;
  float* p;

  float density;
  float spacing;

  float* generators;
}Grid;

Grid grid_new_uniform(size_t size_x, size_t size_y,float value, float density);
Grid grid_new_random(size_t size_x, size_t size_y, float min, float max, float density);

void render_grid(const Grid* grid, int screen_width, int screen_height);

float get_s(const Grid* grid, int i, int j);

void make_incompressible(Grid* grid, float dt);
void add_gravity(Grid* grid, float dt);
void advent_velocity(Grid* grid, float dt);
void advance_grid(Grid* grid, float dt);
