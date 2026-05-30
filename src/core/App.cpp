#include "core/App.h"
#include "scenes/main_menu/MainMenuScene.h"
#include "scenes/playing/PlayingScene.h"
#include "scenes/lobby/LobbyScene.h"

#include <memory>

App::App() {
    // Start with main menu
    currentScene_ = createScene(SceneType::MAIN_MENU);
}

std::unique_ptr<Scene> App::createScene(SceneType type) {
    switch (type) {
        case SceneType::MAIN_MENU:
            return std::make_unique<MainMenuScene>();
        case SceneType::PLAYING_PVP:
            return std::make_unique<PlayingScene>(PlayMode::PvP, false);
        case SceneType::PLAYING_PVE:
            return std::make_unique<PlayingScene>(PlayMode::PvE, false);
        case SceneType::PLAYING_LOADED_PVP:
            return std::make_unique<PlayingScene>(PlayMode::PvP, true);
        case SceneType::PLAYING_LOADED_PVE:
            return std::make_unique<PlayingScene>(PlayMode::PvE, true);
        case SceneType::LOBBY:
            return std::make_unique<LobbyScene>();
        default:
            return std::make_unique<MainMenuScene>();
    }
}

void App::changeScene(SceneType type) {
    // Đặt cờ để đổi cảnh vào cuối frame, tránh xoá vùng nhớ khi đang chạy update
    pendingSceneType_ = type;
    isSceneChangePending_ = true;
}

void App::changeSceneWithInstance(std::unique_ptr<Scene> scene) {
    pendingSceneInstance_ = std::move(scene);
    isSceneChangePending_ = true;
}

void App::quit() {
    shouldClose_ = true;
}

void App::run() {
    while (!WindowShouldClose() && !shouldClose_) {
        // Đổi cảnh nếu có yêu cầu
        if (isSceneChangePending_) {
            if (pendingSceneInstance_) {
                currentScene_ = std::move(pendingSceneInstance_);
            } else {
                currentScene_ = createScene(pendingSceneType_);
            }
            isSceneChangePending_ = false;
        }

        // 1. Update logic của Scene hiện tại
        if (currentScene_) {
            currentScene_->update(this);
        }

        // 2. Render Scene hiện tại
        BeginDrawing();
        ClearBackground(RAYWHITE);
        if (currentScene_) {
            currentScene_->render();
        }
        EndDrawing();
    }
}
