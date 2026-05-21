#include "ui/Button.h"

namespace ui {

Button::Button(Rectangle bounds, const std::string& text, const ButtonStyle& style)
    : bounds_(bounds), text_(text), state_(ButtonState::Normal), style_(style) {}

void Button::update(Vector2 mousePos) {
    if (state_ == ButtonState::Disabled) {
        return;
    }

    if (CheckCollisionPointRec(mousePos, bounds_)) {
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            state_ = ButtonState::Pressed;
        } else if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            if (state_ == ButtonState::Pressed && onClick_) {
                onClick_();
            }
            state_ = ButtonState::Hovered;
        } else {
            state_ = ButtonState::Hovered;
        }
    } else {
        state_ = ButtonState::Normal;
    }
}

bool Button::isClicked() const {
    return state_ == ButtonState::Hovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), bounds_);
}

void Button::draw() const {
    Color fill, border;
    
    switch (state_) {
        case ButtonState::Hovered:
            fill = style_.hoverColor;
            border = style_.borderHoverColor;
            break;
        case ButtonState::Pressed:
            fill = style_.pressedColor;
            border = style_.borderHoverColor;
            break;
        case ButtonState::Disabled:
            fill = Fade(style_.baseColor, 0.5f);
            border = Fade(style_.borderColor, 0.5f);
            break;
        case ButtonState::Normal:
        default:
            fill = style_.baseColor;
            border = style_.borderColor;
            break;
    }

    DrawRectangleRounded(bounds_, style_.roundness, style_.segments, fill);
    DrawRectangleRoundedLinesEx(bounds_, style_.roundness, style_.segments, style_.borderWidth, border);

    int textWidth = MeasureText(text_.c_str(), style_.fontSize);
    int textX = bounds_.x + (bounds_.width - textWidth) / 2;
    int textY = bounds_.y + (bounds_.height - style_.fontSize) / 2;
    
    Color currentTextColor = (state_ == ButtonState::Disabled) ? Fade(style_.textColor, 0.5f) : style_.textColor;
    DrawText(text_.c_str(), textX, textY, style_.fontSize, currentTextColor);
}

} // namespace ui
