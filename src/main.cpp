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

  const char *appDir = GetApplicationDirectory();
  if (appDir != nullptr && appDir[0] != '\0') {
    ChangeDirectory(appDir);
  }

  SetTargetFPS(60);
  Game game;
  ChessView view;
  ChessController controller(game, view);

  if (!view.LoadAssets()) {
    std::fprintf(stderr, "Failed to load chess assets: %s\n",
                 view.getLastAssetError().c_str());
    while (!WindowShouldClose()) {
      BeginDrawing();
      ClearBackground({20, 20, 20, 255});
      DrawText("Failed to load assets.", 50, 220, 24, {255, 80, 80, 255});
      DrawText(view.getLastAssetError().c_str(), 20, 255, 16,
               {230, 230, 230, 255});
      DrawText("Make sure build/assets/images exists.", 50, 290, 20,
               {230, 230, 230, 255});
      DrawText("Press ESC to close.", 50, 320, 20, {230, 230, 230, 255});
      EndDrawing();
    }
    CloseWindow();
    return 1;
  }

  controller.run();

  CloseWindow();
  return 0;
}
