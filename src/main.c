#include "raylib.h"
#include "render.h"
#include <bits/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (450)

#define WINDOW_TITLE "Window title"

int main(void) {

  srand(time(NULL));
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);
  SetTargetFPS(60);

  Grid grid = grid_new_uniform(200,200, 0.0, 1.0, 200.0);

  clock_t begin, end;
  begin = clock();

  Vector2 ball_pos = {.x = 0.0, .y = 0.0};
  float ball_r = 8.0;

  bool mouse_pressed = false;
  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(RAYWHITE);

    draw_grid(&grid);

    EndDrawing();

    end = clock();

    float dt = ((float)(end - begin)) / (CLOCKS_PER_SEC);
    if (mouse_pressed) {
      Vector2 mouse_pos = GetMousePosition();
      Vector2 new_pos = {.x = mouse_pos.x / GetScreenWidth(),
                         .y = mouse_pos.y / GetScreenHeight()};

      move_ball(&grid, ball_pos, new_pos, ball_r, dt);


      ball_pos = new_pos;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 mouse_pos = GetMousePosition();
      Vector2 new_pos = {.x = mouse_pos.x / GetScreenWidth(),
                         .y = mouse_pos.y / GetScreenHeight()};

      move_ball(&grid, new_pos, new_pos, ball_r, dt);

      ball_pos = new_pos;
      mouse_pressed = true;
    } else if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      mouse_pressed = false;
    }



    advance_grid(&grid, dt);

    begin = end;
  }

  CloseWindow();

  return 0;
}
