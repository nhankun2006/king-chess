#include "scenes/main_menu/MenuController.h"
#include "scenes/main_menu/MainMenuScene.h"
#include "config/UIConfig.h"
#include <raylib.h>

MenuController::MenuController(MenuModel* model, MenuView* view)
    : model_(model), view_(view) {}

static Rectangle makeButton(float centerX, float centerY, int idx, float btnWidth, float btnHeight, float btnGap) {
    float y = centerY + idx * btnGap - btnHeight * 0.8f;
    return {centerX - btnWidth / 2.0f, y, btnWidth, btnHeight};
}

void MenuController::update(SceneManager* manager) {
    Vector2 mousePos = GetMousePosition();
    bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    ui::AutoLayout::Metrics metrics = ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
    float scale = metrics.uiScale;

    float centerX = GetScreenWidth() / 2.0f;
    float centerY = GetScreenHeight() / 2.0f;

    float btnWidth = 240.0f * scale;
    float btnHeight = 56.0f * scale;
    float btnGap = 80.0f * scale;

    if (model_->state == MenuState::MAIN) {
        Rectangle b0 = makeButton(centerX, centerY, 0, btnWidth, btnHeight, btnGap);
        Rectangle b1 = makeButton(centerX, centerY, 1, btnWidth, btnHeight, btnGap);
        if (clicked) {
            if (CheckCollisionPointRec(mousePos, b0)) {
                model_->selectedMode = PlayMode::PvP;
                model_->state = MenuState::SELECT_MODE;
            } else if (CheckCollisionPointRec(mousePos, b1)) {
                model_->selectedMode = PlayMode::PvE;
                model_->state = MenuState::SELECT_MODE;
            }
        }
    } else {
        Rectangle n = makeButton(centerX, centerY, 0, btnWidth, btnHeight, btnGap);
        Rectangle l = makeButton(centerX, centerY, 1, btnWidth, btnHeight, btnGap);
        Rectangle back = {centerX - btnWidth / 2.0f, centerY + 100.0f * scale, btnWidth, btnHeight};
        if (clicked) {
            if (CheckCollisionPointRec(mousePos, n)) {
                if (model_->selectedMode == PlayMode::PvP) manager->changeScene(SceneType::PLAYING_PVP);
                else manager->changeScene(SceneType::PLAYING_PVE);
            } else if (CheckCollisionPointRec(mousePos, l)) {
                manager->changeScene(SceneType::PLAYING_LOADED);
            } else if (CheckCollisionPointRec(mousePos, back)) {
                model_->state = MenuState::MAIN;
            }
        }
    }
}
