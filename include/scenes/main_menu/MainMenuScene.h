#ifndef MAINMENUSCENE_H
#define MAINMENUSCENE_H

#include "core/Scene.h"
#include "core/SceneManager.h"
#include <raylib.h>
#include <memory>
#include "MenuModel.h"
#include "MenuView.h"
#include "MenuController.h"

#include "scenes/PlayMode.h"

// Simple enums used by menu and playing scenes
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
