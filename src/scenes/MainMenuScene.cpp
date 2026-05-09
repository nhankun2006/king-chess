#include "scenes/MainMenuScene.h"
#include "App.h"
#include "scenes/PlayingScene.h" 

MainMenuScene::MainMenuScene() {
    float centerX = 800 / 2.0f; // Tạm thời
    float centerY = 600 / 2.0f;

    pvpBtn_ = {centerX - 100, centerY - 60, 200, 50};
    pveBtn_ = {centerX - 100, centerY + 20, 200, 50};

    newGameBtn_  = {centerX - 100, centerY - 60, 200, 50};
    loadGameBtn_ = {centerX - 100, centerY + 20, 200, 50};
    backBtn_     = {centerX - 100, centerY + 100, 200, 50};
}

void MainMenuScene::update(App* app) {
    Vector2 mousePos = GetMousePosition();
    bool clicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    // Cập nhật lại vị trí nút nếu màn hình thay đổi kích thước
    float centerX = GetScreenWidth() / 2.0f;
    float centerY = GetScreenHeight() / 2.0f;
    pvpBtn_ = {centerX - 100, centerY - 60, 200, 50};
    pveBtn_ = {centerX - 100, centerY + 20, 200, 50};
    newGameBtn_  = {centerX - 100, centerY - 60, 200, 50};
    loadGameBtn_ = {centerX - 100, centerY + 20, 200, 50};
    backBtn_     = {centerX - 100, centerY + 100, 200, 50};

    if (state_ == MenuState::MAIN) {
        updateMainState(app, mousePos, clicked);
    } else if (state_ == MenuState::SELECT_MODE) {
        updateSelectModeState(app, mousePos, clicked);
    }
}

void MainMenuScene::updateMainState(App* app, Vector2 mousePos, bool clicked) {
    if (clicked) {
        if (CheckCollisionPointRec(mousePos, pvpBtn_)) {
            selectedMode_ = PlayMode::PvP;
            state_ = MenuState::SELECT_MODE;
        } else if (CheckCollisionPointRec(mousePos, pveBtn_)) {
            selectedMode_ = PlayMode::PvE;
            state_ = MenuState::SELECT_MODE;
        }
    }
}

void MainMenuScene::updateSelectModeState(App* app, Vector2 mousePos, bool clicked) {
    if (clicked) {
        if (CheckCollisionPointRec(mousePos, newGameBtn_)) {
            app->changeScene(new PlayingScene(selectedMode_, false));
        } else if (CheckCollisionPointRec(mousePos, loadGameBtn_)) {
            app->changeScene(new PlayingScene(selectedMode_, true)); // true = load save
        } else if (CheckCollisionPointRec(mousePos, backBtn_)) {
            state_ = MenuState::MAIN;
        }
    }
}

void MainMenuScene::render() {
    ClearBackground(DARKGRAY);

    Vector2 mousePos = GetMousePosition();

    // Helper vẽ nút
    auto drawButton = [&](Rectangle rect, const char* text) {
        bool hover = CheckCollisionPointRec(mousePos, rect);
        DrawRectangleRec(rect, hover ? LIGHTGRAY : GRAY);
        DrawRectangleLinesEx(rect, 2.0f, BLACK);
        int textW = MeasureText(text, 20);
        DrawText(text, rect.x + (rect.width - textW) / 2, rect.y + 15, 20, BLACK);
    };

    DrawText("KING CHESS", GetScreenWidth() / 2 - MeasureText("KING CHESS", 40) / 2, GetScreenHeight() / 2 - 150, 40, WHITE);

    if (state_ == MenuState::MAIN) {
        drawButton(pvpBtn_, "Player vs Player");
        drawButton(pveBtn_, "Player vs Bot");
    } else if (state_ == MenuState::SELECT_MODE) {
        const char* modeText = (selectedMode_ == PlayMode::PvP) ? "Mode: PvP" : "Mode: PvE";
        DrawText(modeText, GetScreenWidth() / 2 - MeasureText(modeText, 20) / 2, GetScreenHeight() / 2 - 100, 20, YELLOW);
        
        drawButton(newGameBtn_, "New Game");
        drawButton(loadGameBtn_, "Load Profile");
        drawButton(backBtn_, "Back");
    }
}
