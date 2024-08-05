#include "fluid.hpp"

#include <algorithm>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <vector>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

Fluid::Fluid(size_t grid_x, size_t grid_y, float h)
    : grid_x(grid_x), grid_y(grid_y), height(grid_x * h), width(grid_y * h),
      h(h) {

  particles = std::vector<Particle>();
  grid = new Cell[grid_x * grid_y];

  for (int i = 0; i < grid_x * grid_y; i++) {
    grid[i].particles = NULL;
    grid[i].len = 0;
  }
}

Fluid::~Fluid() { delete grid; }

int get_cell_idx(Fluid *f, Vector2 pos) {
  int x = (int)(pos.x / f->h);
  int y = (int)(pos.y / f->h);
  return y * f->grid_x + x;
}

int Fluid::get_grid_idx(Vector2 pos) {
  int x = (int)(pos.x / h);
  int y = (int)(pos.y / h);
  return x * grid_x + y;
}

void Fluid::sort_particles() {
  std::sort(particles.begin(), particles.end(),
            [this](const Particle &a, const Particle &b) {
              return this->get_grid_idx(a.x) < this->get_grid_idx(b.x);
            });

  int j = 0;
  for (int i = 0; i < grid_x * grid_y; i++) {
    Cell &cell = grid[i];

    cell.particles = &particles[j];
    int last_j = j;
    for (; j < particles.size() && get_grid_idx(particles[j].x) == i; j++, i++)
      ;
    cell.len = j - last_j;
    if (j >= particles.size()) {
      break;
    }
  }
}

void Fluid::apply_gravity(float dt){
  for (int i = 0; i < particles.size(); i++){
    Particle &p = particles[i];
    p.v.y += gravity *  dt;
    p.x.x += p.v.x * dt;
    p.x.y += p.v.y * dt;
  }
}

void Fluid::run_sim(float dt) {
  //sort_particles();
  apply_gravity(dt);
}
