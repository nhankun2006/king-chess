#ifndef UI_BUTTON_H
#define UI_BUTTON_H

#include <raylib.h>
#include <string>
#include <functional>

namespace ui {

enum class ButtonState {
    Normal,
    Hovered,
    Pressed,
    Disabled
};

struct ButtonStyle {
    Color baseColor = {48, 58, 78, 255};
    Color hoverColor = {72, 86, 112, 255};
    Color pressedColor = {38, 48, 68, 255};
    Color textColor = {235, 235, 235, 255};
    Color borderColor = {90, 105, 130, 255};
    Color borderHoverColor = {132, 152, 190, 255};
    float roundness = 0.25f;
    int segments = 8;
    float borderWidth = 2.0f;
    int fontSize = 20;
};

class Button {
private:
    Rectangle bounds_;
    std::string text_;
    ButtonState state_;
    ButtonStyle style_;
    std::function<void()> onClick_;

public:
    Button(Rectangle bounds, const std::string& text, const ButtonStyle& style = ButtonStyle{});

    void update(Vector2 mousePos);
    void draw() const;

    bool isClicked() const;
    ButtonState getState() const { return state_; }

    void setText(const std::string& text) { text_ = text; }
    void setBounds(Rectangle bounds) { bounds_ = bounds; }
    void setDisabled(bool disabled) { state_ = disabled ? ButtonState::Disabled : ButtonState::Normal; }
    void setOnClick(std::function<void()> callback) { onClick_ = callback; }
};

} // namespace ui

#endif // UI_BUTTON_H
