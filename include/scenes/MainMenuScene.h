#ifndef MAINMENUSCENE_H
#define MAINMENUSCENE_H

#include "scenes/Scene.h"
#include <raylib.h>

// Trạng thái cục bộ của Menu
enum class MenuState {
    MAIN,
    SELECT_MODE // Khi đã chọn PvP/PvE, chuyển qua chọn New/Load
};

enum class PlayMode {
    PvP,
    PvE
};

class MainMenuScene : public Scene {
private:
    MenuState state_ = MenuState::MAIN;
    PlayMode selectedMode_ = PlayMode::PvP;

    // Các khu vực nút bấm (Rectangles)
    Rectangle pvpBtn_;
    Rectangle pveBtn_;
    
    Rectangle newGameBtn_;
    Rectangle loadGameBtn_;
    Rectangle backBtn_;

    // Tách nhỏ logic cập nhật theo State của Menu
    void updateMainState(App* app, Vector2 mousePos, bool clicked);
    void updateSelectModeState(App* app, Vector2 mousePos, bool clicked);

public:
    MainMenuScene();
    ~MainMenuScene() override = default;

    // Kế thừa từ Scene
    void update(App* app) override;
    void render() override;
};

#endif // MAINMENUSCENE_H
