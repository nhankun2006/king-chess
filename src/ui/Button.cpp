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

    if (icon_.id != 0) {
        float minDimension = (bounds_.width < bounds_.height) ? bounds_.width : bounds_.height;
        float iconSize = minDimension * 0.6f; // Scale icon to fit inside button
        Rectangle iconSrc = {0.0f, 0.0f, static_cast<float>(icon_.width), static_cast<float>(icon_.height)};
        Rectangle iconDst = {
            bounds_.x + (bounds_.width - iconSize) * 0.5f,
            bounds_.y + (bounds_.height - iconSize) * 0.5f,
            iconSize, iconSize
        };
        DrawTexturePro(icon_, iconSrc, iconDst, {0.0f, 0.0f}, 0.0f, WHITE);
    } else if (!text_.empty()) {
        int textWidth = MeasureText(text_.c_str(), style_.fontSize);
        int textX = bounds_.x + (bounds_.width - textWidth) / 2;
        int textY = bounds_.y + (bounds_.height - style_.fontSize) / 2;
        
        Color currentTextColor = (state_ == ButtonState::Disabled) ? Fade(style_.textColor, 0.5f) : style_.textColor;
        DrawText(text_.c_str(), textX, textY, style_.fontSize, currentTextColor);
    }

    if (!tooltip_.empty() && (state_ == ButtonState::Hovered || state_ == ButtonState::Pressed)) {
        int tooltipFontSize = static_cast<int>(16.0f * (GetScreenHeight() / 720.0f)); // Approximate UI scale
        if (tooltipFontSize < 10) tooltipFontSize = 10;
        int textWidth = MeasureText(tooltip_.c_str(), tooltipFontSize);
        float buttonCenterX = bounds_.x + bounds_.width * 0.5f;
        int textX = static_cast<int>(buttonCenterX - textWidth * 0.5f);
        int textY = static_cast<int>(bounds_.y - tooltipFontSize * 1.5f);

        // Clamp horizontal position to stay on-screen
        int screenW = GetScreenWidth();
        if (textX < 0) textX = 0;
        if (textX + textWidth > screenW) textX = screenW - textWidth;

        // Flip below button if tooltip would go off the top edge
        if (textY < 0) {
            textY = static_cast<int>(bounds_.y + bounds_.height + tooltipFontSize * 0.5f);
        }
        // Final vertical clamp
        int screenH = GetScreenHeight();
        if (textY + tooltipFontSize > screenH) textY = screenH - tooltipFontSize;

        DrawText(tooltip_.c_str(), textX, textY, tooltipFontSize, {150, 160, 170, 255}); // Muted text color for tooltip
    }
}

} // namespace ui
