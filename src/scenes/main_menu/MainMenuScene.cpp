#include "scenes/main_menu/MainMenuScene.h"

MainMenuScene::MainMenuScene() {
    model_ = std::make_unique<MenuModel>();
    view_ = std::make_unique<MenuView>();
    controller_ = std::make_unique<MenuController>(model_.get(), view_.get());
}

void MainMenuScene::update(SceneManager* manager) {
    // Controller handles input and model changes and can request scene changes
    if (controller_) controller_->update(manager);
}

void MainMenuScene::render() {
    if (view_ && model_) view_->render(*model_);
}
