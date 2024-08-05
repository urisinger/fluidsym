#include <stddef.h>
#include <raylib.h>
#include <vector>

struct Particle{
  Vector2 x;
  Vector2 v;
  float d;
};

struct Cell{
  Particle* particles;
  size_t len;
};

struct Fluid{
  float ball_size = 10.0;
  float gravity = 10.0;

  float height;
  float width;
  float h;

  size_t grid_x;
  size_t grid_y;

  std::vector<Particle> particles;
  Cell* grid;

  Fluid(size_t grid_x, size_t grid_y,float h);
  ~Fluid();

  void run_sim(float dt);

  void apply_gravity(float dt);

  int get_grid_idx(Vector2 pos);
  void sort_particles();
};
