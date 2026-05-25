#ifndef UI_DIALOG_H
#define UI_DIALOG_H

#include <raylib.h>
#include <string>

namespace ui {

struct DialogStyle {
    Color backgroundColor = {32, 38, 50, 255};
    Color borderColor = {110, 124, 150, 255};
    Color titleColor = {235, 235, 235, 255};
    Color bodyColor = {195, 205, 220, 255};
    Color overlayColor = {0, 0, 0, 120};
    
    float roundness = 0.16f;
    int segments = 12;
    float borderWidth = 2.0f;
    
    int titleFontSize = 24;
    int bodyFontSize = 16;
};

class DialogBox {
private:
    Rectangle bounds_;
    std::string title_;
    std::string body_;
    DialogStyle style_;

public:
    DialogBox(Rectangle bounds, const std::string& title, const std::string& body = "", const DialogStyle& style = DialogStyle{});

    // Draws the overlay and the dialog box itself
    void draw() const;

    void setTitle(const std::string& title) { title_ = title; }
    void setBody(const std::string& body) { body_ = body; }
    void setBounds(Rectangle bounds) { bounds_ = bounds; }
    
    Rectangle getBounds() const { return bounds_; }
};

} // namespace ui

#endif // UI_DIALOG_H
