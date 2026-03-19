#include <cstdio>

#define Color RaylibColor
#include <raylib.h>
#undef Color

#include "ChessControllder.h"
#include "ChessView.h"
#include "Game.h"

int main() {
  InitWindow(512, 512, "King Chess");
  if (!IsWindowReady()) {
    std::fprintf(stderr, "InitWindow failed\n");
    return 1;
  }

  SetTargetFPS(60);
  Game game;
  ChessView view;
  ChessController controller(game, view);

  if (!view.LoadAssets()) {
    std::fprintf(stderr, "Failed to load chess assets\n");
    CloseWindow();
    return 1;
  }

  controller.run();

  CloseWindow();
  return 0;
}
