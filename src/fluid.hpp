#include <stddef.h>
#include <raylib.h>
#include <vector>
#include <span>

struct Particle{
  Vector2 x;
  Vector2 predicted_x;
  Vector2 v;
  float d;
};

struct Fluid{
  float ball_size = 10.0;
  float gravity = 10.0;

  float height;
  float width;
  float h;

  float k = 1.0;
  float collision_dampening = 0.5;
  float desired_density;

  size_t grid_x;
  size_t grid_y;

  std::vector<Particle> particles;
  std::vector<std::span<Particle>> grid;

  Fluid(size_t grid_x, size_t grid_y,float height, float desired_density);

  float smoothing_kernel(float r);
  float smoothing_kernel_derivitive(float r);

  float compute_density(const Particle& p1);
  Vector2 compute_pressure(const Particle& p1, float dt);

  void run_sim(float dt);

  int get_grid_idx(Vector2 pos);
  void sort_particles();
};
