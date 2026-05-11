#include "scenes/MenuController.h"
#include "scenes/MainMenuScene.h"
#include "UIConfig.h"
#include <raylib.h>

MenuController::MenuController(MenuModel* model, MenuView* view)
    : model_(model), view_(view) {}

static Rectangle makeButton(float centerX, float centerY, int idx, float btnWidth, float btnHeight, float btnGap) {
    float y = centerY + idx * btnGap - btnHeight * 0.8f + 50.0f * (btnHeight / 56.0f);
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

    if (model_->state == MenuState::HOME) {
        Rectangle b0 = makeButton(centerX, centerY, 0, btnWidth, btnHeight, btnGap);
        Rectangle b1 = makeButton(centerX, centerY, 1, btnWidth, btnHeight, btnGap);
        Rectangle b2 = makeButton(centerX, centerY, 2, btnWidth, btnHeight, btnGap);
        Rectangle creditBtn = { GetScreenWidth() - 150.0f * scale, GetScreenHeight() - 60.0f * scale, 130.0f * scale, 40.0f * scale };
        if (clicked) {
            if (CheckCollisionPointRec(mousePos, b0)) {
                model_->state = MenuState::PLAY_SELECT;
            } else if (CheckCollisionPointRec(mousePos, b1)) {
                model_->state = MenuState::SETTINGS;
            } else if (CheckCollisionPointRec(mousePos, b2)) {
                manager->quit();
            } else if (CheckCollisionPointRec(mousePos, creditBtn)) {
                model_->state = MenuState::CREDITS;
            }
        }
    } else if (model_->state == MenuState::PLAY_SELECT) {
        Rectangle b0 = makeButton(centerX, centerY, 0, btnWidth, btnHeight, btnGap);
        Rectangle b1 = makeButton(centerX, centerY, 1, btnWidth, btnHeight, btnGap);
        Rectangle back = { 20.0f * scale, GetScreenHeight() - 40.0f * scale - 20.0f * scale, 80.0f * scale, 40.0f * scale };
        if (clicked) {
            if (CheckCollisionPointRec(mousePos, b0)) {
                model_->selectedMode = PlayMode::PvP;
                model_->state = MenuState::SELECT_MODE;
            } else if (CheckCollisionPointRec(mousePos, b1)) {
                model_->selectedMode = PlayMode::PvE;
                model_->state = MenuState::SELECT_MODE;
            } else if (CheckCollisionPointRec(mousePos, back)) {
                model_->state = MenuState::HOME;
            }
        }
    } else if (model_->state == MenuState::SELECT_MODE) {
        Rectangle n = makeButton(centerX, centerY, 0, btnWidth, btnHeight, btnGap);
        Rectangle l = makeButton(centerX, centerY, 1, btnWidth, btnHeight, btnGap);
        Rectangle back = { 20.0f * scale, GetScreenHeight() - 40.0f * scale - 20.0f * scale, 80.0f * scale, 40.0f * scale };
        if (clicked) {
            if (CheckCollisionPointRec(mousePos, n)) {
                model_->state = MenuState::SELECT_TIMER;
            } else if (CheckCollisionPointRec(mousePos, l)) {
                if (model_->selectedMode == PlayMode::PvP) manager->changeScene(SceneType::PLAYING_LOADED_PVP);
                else manager->changeScene(SceneType::PLAYING_LOADED_PVE);
            } else if (CheckCollisionPointRec(mousePos, back)) {
                model_->state = MenuState::PLAY_SELECT;
            }
        }
    } else if (model_->state == MenuState::SELECT_TIMER) {
        float arrowSize = 60.0f * scale;
        float boxWidth = 200.0f * scale;
        Rectangle leftArrowBtn = { centerX - boxWidth/2 - arrowSize - 20.0f*scale, centerY - arrowSize/2 + 50.0f * (btnHeight / 56.0f), arrowSize, arrowSize };
        Rectangle rightArrowBtn = { centerX + boxWidth/2 + 20.0f*scale, centerY - arrowSize/2 + 50.0f * (btnHeight / 56.0f), arrowSize, arrowSize };
        Rectangle startBtn = makeButton(centerX, centerY, 1.5f, btnWidth, btnHeight, btnGap);
        Rectangle back = { 20.0f * scale, GetScreenHeight() - 40.0f * scale - 20.0f * scale, 80.0f * scale, 40.0f * scale };
        
        const int timeOptions[] = {1, 5, 10, 15, 30};
        const int numOptions = 5;

        if (clicked) {
            auto goPlaying = [&]() {
                g_targetTimeControl = model_->selectedTimer;
                if (model_->selectedMode == PlayMode::PvP) manager->changeScene(SceneType::PLAYING_PVP);
                else manager->changeScene(SceneType::PLAYING_PVE);
            };

            if (CheckCollisionPointRec(mousePos, leftArrowBtn)) {
                int currIdx = 0;
                for (int i=0; i<numOptions; ++i) if (timeOptions[i] == model_->selectedTimer) currIdx = i;
                if (currIdx > 0) model_->selectedTimer = timeOptions[currIdx - 1];
            } else if (CheckCollisionPointRec(mousePos, rightArrowBtn)) {
                int currIdx = 0;
                for (int i=0; i<numOptions; ++i) if (timeOptions[i] == model_->selectedTimer) currIdx = i;
                if (currIdx < numOptions - 1) model_->selectedTimer = timeOptions[currIdx + 1];
            } else if (CheckCollisionPointRec(mousePos, startBtn)) {
                goPlaying();
            } else if (CheckCollisionPointRec(mousePos, back)) {
                model_->state = MenuState::SELECT_MODE;
            }
        }
    } else if (model_->state == MenuState::SETTINGS || model_->state == MenuState::CREDITS) {
        Rectangle back = { 20.0f * scale, GetScreenHeight() - 40.0f * scale - 20.0f * scale, 80.0f * scale, 40.0f * scale };
        if (clicked && CheckCollisionPointRec(mousePos, back)) {
            model_->state = MenuState::HOME;
        }
    }
}
