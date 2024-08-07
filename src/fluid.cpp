#include "fluid.hpp"

#include <algorithm>
#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <vector>
#include <cmath>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

Fluid::Fluid(size_t grid_x, size_t grid_y, float height, float desired_density)
    : grid_x(grid_x), grid_y(grid_y), height(grid_x * h), width(grid_y * h),
      h(height / grid_y), desired_density(desired_density){

  particles = std::vector<Particle>();
  grid = std::vector(grid_x * grid_y, std::span<Particle>(particles.data(), 0));
}

int Fluid::get_grid_idx(Vector2 pos) {
  int x = (int)(pos.x / h);
  int y = (int)(pos.y / h);
  return y * grid_x + x;
}

void Fluid::sort_particles() {
  std::sort(particles.begin(), particles.end(),
            [this](const Particle &a, const Particle &b) {
              return this->get_grid_idx(a.x) < this->get_grid_idx(b.x);
            });

  int j = 0;
  for (int i = 0; i < grid_x * grid_y; i++) {
    int last_j = j;
    for (; j < particles.size() && get_grid_idx(particles[j].x) == i; j++);
    grid[i] = std::span(particles.data() + last_j, j - last_j);

    if (j >= particles.size()) {
      break;
    }
  }
}

float Fluid::smoothing_kernel(float r){
    if (r > h * h) {
        return 0;
    }
    float v = PI * std::pow(h, 8) / 4;
    float value = h * h - r;
    return value * value * value / v;
}

float Fluid::smoothing_kernel_derivitive(float r) {
    if (r > h) {
        return 0;
    }
    float v = PI * std::pow(h, 5) / 10;
    float value = h-r;
    return value * value / v;
}

const std::pair<int, int> neighbor_offsets[9] = {
    {1,-1},
    {1, 0},
    {1, 1},
    {0, 1},
    {0, 0},
    {0, -1},
    {-1, -1},
    {-1, 0},
    {-1, 1}
};

float Fluid::compute_density(const Particle& p1) {
    float d = 0.0;

    for (auto [i, j] : neighbor_offsets) {

        int x = (int)(p1.x.x / h) + j;
        int y = (int)(p1.x.y / h) + i;

        if (x < 0 || x >= grid_x || y < 0 || y >= grid_y) {
            continue;
        }

        for (Particle& p2 : grid[y * grid_x + x]) {
            if (&p1 == &p2) {
                continue;
            }
            float r = Vector2DistanceSqr(p1.x, p2.x);


            d += smoothing_kernel(r);
            
        }
    }
    return d;
}

Vector2 Fluid::compute_pressure(const Particle& p1, float dt){
    Vector2 v = { 0.0, 0.0 };

    for (auto [i, j] : neighbor_offsets) {

        int x = (int)(p1.x.x / h) + j;
        int y = (int)(p1.x.y / h) + i;

        if (x < 0 || x >= grid_x || y < 0 || y >= grid_y) {
            continue;
        }

        for (Particle& p2 : grid[y * grid_x + x]) {
            if (&p1 == &p2) {
                continue;
            }
            float r = Vector2Distance(p1.x, p2.x);

            if (r < h) {

                float slope = smoothing_kernel_derivitive(r);
                float pressure = p2.d + p1.d - 2 * desired_density;
                pressure *= k * slope / 2 * p2.d;
                Vector2 dir = Vector2Subtract(p1.x, p2.x);
                dir = Vector2Normalize(dir);


                
                v = Vector2Add(v, Vector2Scale(dir, pressure / p1.d));
            }
        }
    }
    return v;
}


void Fluid::run_sim(float dt) {
    for (Particle& p : particles) {
        p.v.y += gravity * dt;
        p.x.x += p.v.x * dt;
        p.x.y += p.v.y * dt;
    }

    for (Particle& p : particles) {
        p.d = compute_density(p);
    }

    for (Particle& p : particles) {
        p.v = Vector2Add(p.v ,Vector2Scale(compute_pressure(p, dt), dt));
    }

    for (Particle& p : particles) {

        if (p.x.x < h) {
            p.x.x = h;
            p.v.x *= -collision_dampening;
        }
        if (p.x.y < h) {
            p.x.y = h;
            p.v.y *= -collision_dampening;
        }
        if (p.x.x > grid_x * h - h) {
            p.x.x = grid_x * h - h;
            p.v.x *= -collision_dampening;
        }
        if (p.x.y > grid_y * h - h) {
            p.x.y = grid_y * h - h;
            p.v.y *= -collision_dampening;
        }
    }
    sort_particles();
}
