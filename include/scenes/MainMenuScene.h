#ifndef MAINMENUSCENE_H
#define MAINMENUSCENE_H

#include "scenes/Scene.h"
#include "scenes/SceneManager.h"
#include <raylib.h>
#include <memory>
#include "scenes/MenuModel.h"
#include "scenes/MenuView.h"
#include "scenes/MenuController.h"

// Simple enums used by menu and playing scenes
enum class PlayMode { PvP, PvE };
enum class MenuState { HOME, PLAY_SELECT, SELECT_MODE, SELECT_TIMER, SETTINGS, CREDITS };

class MainMenuScene : public Scene {
private:
    // MVC components for the menu
    std::unique_ptr<MenuModel> model_;
    std::unique_ptr<MenuView> view_;
    std::unique_ptr<MenuController> controller_;

public:
    MainMenuScene();
    ~MainMenuScene() override = default;

    // Kế thừa từ Scene
    void update(SceneManager* manager) override;
    void render() override;
};

#endif // MAINMENUSCENE_H
