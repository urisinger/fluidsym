#include <stddef.h>
#include <raylib.h>

typedef struct Fluid{
  size_t size_x;
  size_t size_y;

  float* new_v;
  float* new_u;

  float overrelax;
  float* y_velo;
  float* x_velo;
  float* pressure;
  float* s;

  float* old_smoke;
  float* smoke;

  float density;
  float spacing;

  float frame;
}Fluid;

Fluid fluid_new(size_t size_x, size_t size_y,float overrelax,  float height);

void fluid_free(Fluid* f);

float get_s(const Fluid* f, int i, int j);

void make_incompressible(Fluid* f, float dt);
void add_gravity(Fluid* f, float dt);
void advent_velocity(Fluid* f, float dt);
void advance_grid(Fluid* f, float dt);

void move_ball(Fluid *f, Vector2 last_pos, Vector2 next_pos, float r, float dt);
