#include <stddef.h>
#include <raylib.h>

typedef struct Grid{
  size_t size_x;
  size_t size_y;

  float* new_v;
  float* new_u;

  float* y_velo;
  float* x_velo;
  float* pressure;
  float* s;


  float* old_smoke;
  float* smoke;

  float density;
  float spacing;

  float frame;

}Grid;

Grid grid_new_uniform(size_t size_x, size_t size_y,float value, float density, float height);
Grid grid_new_random(size_t size_x, size_t size_y, float min, float max, float density, float height);

float get_s(const Grid* grid, int i, int j);

void make_incompressible(Grid* grid, float dt);
void add_gravity(Grid* grid, float dt);
void advent_velocity(Grid* grid, float dt);
void advance_grid(Grid* grid, float dt);


void move_ball(Grid *grid, Vector2 last_pos, Vector2 next_pos, float r, float dt);
