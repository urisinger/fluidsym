#include "raylib.h"
#include "render.hpp"
#include <bits/time.h>
#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <time.h>

#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

#define SCREEN_WIDTH (1920)
#define SCREEN_HEIGHT (1080)

#define WINDOW_TITLE "Fluidsym"

int main(void) {
  srand(time(NULL));
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(60);

  Fluid fluid = Fluid(50, 50, 100.0);

  Vector2 ball_pos = {.x = 0.0, .y = 0.0};
  float ball_r = 5.0;

  float dt = 0.016f;

  auto begin = std::chrono::high_resolution_clock::now();

  while (!WindowShouldClose()) {
    Rectangle slider_rect = {.x = 0, .y = 0, .width = 100, .height = 40};
    GuiSlider({.x = 0, .y = 0, .width = 100, .height = 20}, "", "ball radius", &fluid.ball_size, 2.0, 20.0);
    GuiSlider({.x = 0, .y = 20, .width = 100, .height = 20}, "", "gravity", &fluid.gravity, 5.0, 20.0);

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

        while (time_diff.count() > dt) {
            fluid.run_sim(dt);
            time_diff -= std::chrono::duration<float>(dt);
            begin = now;
        }
    BeginDrawing();

    ClearBackground(RAYWHITE);

    draw_fluid(fluid);

    EndDrawing();




  }

  CloseWindow();

  return 0;
}
