#include <cstdio>

#include <raylib.h>

#include "UIConfig.h"
#include "App.h"

int main() {
  SetConfigFlags(FLAG_VSYNC_HINT);
  
  const char *appDir = GetApplicationDirectory();
  if (appDir != nullptr && appDir[0] != '\0') {
    ChangeDirectory(appDir);
  }

  int initWidth = ui::Window::kInitialWidth;
  int initHeight = ui::Window::kInitialHeight;
  FILE* f = fopen("settings.ini", "r");
  if (f) {
    fscanf(f, "%d %d", &initWidth, &initHeight);
    fclose(f);
  }

  InitWindow(initWidth, initHeight, "King Chess");
  if (!IsWindowReady()) {
    std::fprintf(stderr, "InitWindow failed\n");
    return 1;
  }

  InitAudioDevice();

  SetTargetFPS(ui::Window::kTargetFps);

  // Use App / SceneManager based flow
  App app;
  app.run();

  CloseAudioDevice();
  CloseWindow();
  return 0;
}
