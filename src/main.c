#include "raylib.h"
#include "grid.h"
#include <bits/time.h>
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

  Grid grid = grid_new_uniform(100, 100,  10.0,100.0);

  clock_t begin, end;
  begin = clock();
  while (!WindowShouldClose()) {

    BeginDrawing();

    ClearBackground(RAYWHITE);

    render_grid(&grid, GetRenderWidth(), GetRenderHeight()); 

    EndDrawing();

    end = clock();

    float dt = ((float)(end - begin)) / (CLOCKS_PER_SEC / 100);
    
    advance_grid(&grid, dt);

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
      Vector2 pos = GetMousePosition();
      Vector2 tile_pos = {.x = (pos.x * grid.size_x) / GetScreenWidth(), .y = (pos.y * grid.size_y) / GetScreenHeight()};
      
      grid.u[(int)tile_pos.y * grid.size_x  + (int)tile_pos.x] = 1000.0;
    }

    printf("%f\n", dt);
    
    begin = end;
  }

  CloseWindow();

  return 0;
}
