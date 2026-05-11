#include "core/App.h"
#include "scenes/main_menu/MainMenuScene.h"
#include "scenes/playing/PlayingScene.h"

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
        case SceneType::PLAYING_LOADED:
            return std::make_unique<PlayingScene>(PlayMode::PvP, true);
        default:
            return std::make_unique<MainMenuScene>();
    }
}
