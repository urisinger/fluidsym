#include "render.hpp"
#include "raylib.h"

void draw_ball(Vector2 pos, float r) {
  int screen_width = GetRenderWidth();
  int screen_height = GetRenderHeight();

  Vector2 center = {.x = pos.x * screen_width, .y = pos.y * screen_height};
  DrawCircleV(center, screen_height * r, RAYWHITE);
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

void draw_fluid(const Fluid &f) {
  int screen_width = GetRenderWidth();
  int screen_height = GetRenderHeight();
  
  for (Particle p : f.particles){
    DrawCircle(screen_width * p.x.x / (f.grid_x * f.h), screen_height * p.x.y / (f.grid_y * f.h), f.ball_size, BLUE);
  }
}
