#include "ui/Dialog.h"

namespace ui {

DialogBox::DialogBox(Rectangle bounds, const std::string& title, const std::string& body, const DialogStyle& style)
    : bounds_(bounds), title_(title), body_(body), style_(style) {}

void DialogBox::draw() const {
    // Draw screen overlay (dimming the background)
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), style_.overlayColor);

    // Draw dialog background
    DrawRectangleRounded(bounds_, style_.roundness, style_.segments, style_.backgroundColor);
    
    // Draw dialog border
    DrawRectangleRoundedLinesEx(bounds_, style_.roundness, style_.segments, style_.borderWidth, style_.borderColor);

    // Draw Title
    float contentY = bounds_.y + bounds_.height * 0.15f; // Proportional top padding
    if (!title_.empty()) {
        int titleWidth = MeasureText(title_.c_str(), style_.titleFontSize);
        int titleX = bounds_.x + (bounds_.width - titleWidth) / 2;
        int titleY = static_cast<int>(contentY);
        DrawText(title_.c_str(), titleX, titleY, style_.titleFontSize, style_.titleColor);
        contentY += style_.titleFontSize + bounds_.height * 0.08f; // Gap after title
    }

    // Draw Body text if it exists
    if (!body_.empty()) {
        int bodyWidth = MeasureText(body_.c_str(), style_.bodyFontSize);
        int bodyX = bounds_.x + (bounds_.width - bodyWidth) / 2;
        int bodyY = static_cast<int>(contentY);
        DrawText(body_.c_str(), bodyX, bodyY, style_.bodyFontSize, style_.bodyColor);
    }
}

} // namespace ui
