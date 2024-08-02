#include "raylib.h"
#include "render.h"
#include <bits/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH (1920)
#define SCREEN_HEIGHT (1080)

#define WINDOW_TITLE "Window title"

int main(void) {

  srand(time(NULL));
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(60);

  int height = 300;
  float ratio = ((float)GetRenderWidth() / GetRenderHeight());

  Fluid fluid = fluid_new(height, height / ratio, 1.1 ,100.0);

  clock_t begin, end;
  begin = clock();

  Vector2 ball_pos = {.x = 0.0, .y = 0.0};
  float ball_r = 5.0;

  bool mouse_pressed = false;
  while (!WindowShouldClose()) {


    BeginDrawing();

    ClearBackground(RAYWHITE);

    draw_fluid(&fluid);

    EndDrawing();

    end = clock();

    float dt = ((float)(end - begin)) / (CLOCKS_PER_SEC);
    if (mouse_pressed) {
      Vector2 mouse_pos = GetMousePosition();
      Vector2 new_pos = {.x = mouse_pos.x / GetScreenWidth(),
                         .y = mouse_pos.y / GetScreenHeight()};

      move_ball(&fluid, ball_pos, new_pos, ball_r, dt);

      ball_pos = new_pos;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 mouse_pos = GetMousePosition();
      Vector2 new_pos = {.x = mouse_pos.x / GetScreenWidth(),
                         .y = mouse_pos.y / GetScreenHeight()};

      move_ball(&fluid, new_pos, new_pos, ball_r, dt);

      ball_pos = new_pos;
      mouse_pressed = true;
    } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      mouse_pressed = false;
    }

    advance_grid(&fluid, dt);


    begin = end;
  }

  fluid_free(&fluid);

  CloseWindow();

  return 0;
}
