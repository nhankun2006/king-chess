#include "scenes/playing/states/WindowModalInteractionState.h"

#include <cstdio>

#include "config/UIConfig.h"
#include "scenes/playing/ChessController.h"
#include "scenes/playing/states/IdleInteractionState.h"

#include <raylib.h>

bool WindowModalInteractionState::handleInput(ChessController &ctrl) {
  if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
    return false;
  }

  const Vector2 mousePos = GetMousePosition();

  if (ctrl.view_->isWindowSizeDialogCloseClicked(mousePos.x, mousePos.y)) {
    ctrl.windowSizeDialogOpen_ = false;
    ctrl.setState(std::make_unique<IdleInteractionState>());
    return false;
  }

  if (ctrl.view_->isExitToMenuButtonClicked(mousePos.x, mousePos.y)) {
    ctrl.windowSizeDialogOpen_ = false;
    return true; // signal exit to menu
  }

  const int selectedSizeOpt =
      ctrl.view_->getWindowSizeOptionClicked(mousePos.x, mousePos.y);
  if (selectedSizeOpt >= 0 &&
      selectedSizeOpt < ui::Window::kSizePresetCount) {
    const ui::WindowPreset preset = ui::Window::kSizePresets[selectedSizeOpt];
    SetWindowSize(preset.width, preset.height);

    FILE *f = std::fopen("settings.ini", "w");
    if (f != nullptr) {
      std::fprintf(f, "%d %d\n", preset.width, preset.height);
      std::fclose(f);
    }
    ctrl.windowSizeDialogOpen_ = false;
    ctrl.setState(std::make_unique<IdleInteractionState>());
  }

  return false;
}
