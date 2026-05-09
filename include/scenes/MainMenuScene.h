#ifndef MAINMENUSCENE_H
#define MAINMENUSCENE_H

#include "scenes/Scene.h"
#include <memory>

class MenuModel;
class MenuView;
class MenuController;

class MainMenuScene : public Scene {
private:
    std::unique_ptr<MenuModel> model_;
    std::unique_ptr<MenuView> view_;
    std::unique_ptr<MenuController> controller_;

public:
    MainMenuScene();
    ~MainMenuScene() override;

    // Kế thừa từ Scene
    void update(SceneManager* manager) override;
    void render() override;
};

#endif // MAINMENUSCENE_H
