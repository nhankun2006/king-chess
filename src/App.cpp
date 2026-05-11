#include "App.h"
#include "scenes/MainMenuScene.h"
#include "scenes/PlayingScene.h"

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
        default:
            return std::make_unique<MainMenuScene>();
    }
}
