#include "raylib.h"
#include "render.hpp"
#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <time.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

int main(void) {
  srand(time(NULL));
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);    // Window configuration flags
  const int screenWidth = 800;
  const int screenHeight = 450;
  InitWindow(screenWidth, screenWidth, "fluidsim");

  Fluid fluid = Fluid(5, 5, 100.0, 5.0);

  Vector2 ball_pos = {.x = 0.0, .y = 0.0};
  float ball_r = 5.0;

  float dt = 0.016f;

  auto begin = std::chrono::high_resolution_clock::now();

  while (!WindowShouldClose()) {
    Rectangle slider_rect = {.x = 0, .y = 0, .width = 120, .height = 80};
    GuiSlider({.x = 0, .y = 0, .width = 100, .height = 20}, "", "ball radius", &fluid.ball_size, 2.0, 20.0);
    GuiSlider({.x = 0, .y = 20, .width = 100, .height = 20}, "", "gravity", &fluid.gravity, 0.0, 20.0);
    GuiSlider({ .x = 0, .y = 40, .width = 100, .height = 20 }, "", "desired density", &fluid.desired_density, 0.0, 2.0);
    GuiSlider({ .x = 0, .y = 60, .width = 100, .height = 20 }, "", "pressure multiplier", &fluid.k, 0.0, 100.0);

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) &&
        !CheckCollisionPointRec(GetMousePosition(), slider_rect)) {
      Vector2 ball_pos = {.x = ((float)GetMouseX() / GetScreenWidth()) *
                               (fluid.grid_x * fluid.h),
                          .y = ((float)GetMouseY() / GetScreenHeight()) *
                               (fluid.grid_y * fluid.h)};

      Particle p = {.x = ball_pos, .v = {0.0, 0.0}, .d = 0.0};
      fluid.particles.push_back(p);
    }
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> time_diff = now - begin;


        printf("%f \n", time_diff.count());
            fluid.run_sim(time_diff.count());

            begin = now;

    BeginDrawing();

    ClearBackground(RAYWHITE);

    draw_fluid(fluid);

    EndDrawing();




  }

  CloseWindow();

  return 0;
}
