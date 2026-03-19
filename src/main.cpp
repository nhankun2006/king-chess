#include <cstdio>

#define Color RaylibColor
#include <raylib.h>
#undef Color

#include "Game.h"

int main() {
  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(800, 600, "King Chess");
  if (!IsWindowReady()) {
    std::fprintf(stderr, "InitWindow failed\n");
    return 1;
  }

  SetTargetFPS(60);
  std::puts("raylib initialized successfully");

  // ── Initialize chess game ───────────────────────────────────────────
  Game game;
  std::printf("Initial board: %s\n", game.getBoard().toFEN().c_str());
  std::printf("Current turn: %s\n",
              game.getCurrentTurn() == Color::White ? "White" : "Black");

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground({245, 245, 245, 255});
    DrawText("King Chess", 20, 20, 32, {70, 70, 70, 255});
    DrawText("raylib migration baseline", 20, 60, 20, {120, 120, 120, 255});
    EndDrawing();
  }

  CloseWindow();
  return 0;
}
