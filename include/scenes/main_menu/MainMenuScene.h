#ifndef MAINMENUSCENE_H
#define MAINMENUSCENE_H

#include "core/Scene.h"
#include "core/SceneManager.h"
#include "scenes/PlayMode.h"
#include <raylib.h>
#include <memory>
#include "scenes/main_menu/MenuModel.h"
#include "scenes/main_menu/MenuView.h"
#include "scenes/main_menu/MenuController.h"

// Simple enum used by main menu scene
enum class MenuState { MAIN, SELECT_MODE };

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
