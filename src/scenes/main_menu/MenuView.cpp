#include "scenes/main_menu/MenuView.h"
#include "scenes/main_menu/MainMenuScene.h"
#include "config/UIConfig.h"
#include <string>

static Rectangle makeButton(float centerX, float centerY, int idx, float btnWidth, float btnHeight, float btnGap) {
    float y = centerY + idx * btnGap - btnHeight * 0.8f;
    return {centerX - btnWidth / 2.0f, y, btnWidth, btnHeight};
}

static void drawMenuButton(Rectangle buttonRect, bool hovered, const char* text, float scale) {
    if (hovered) {
        const float boost = 2.0f * scale;
        buttonRect.x -= boost;
        buttonRect.y -= boost;
        buttonRect.width += boost * 2.0f;
        buttonRect.height += boost * 2.0f;
    }

    DrawRectangleRounded(buttonRect, ui::IconButtons::kRoundness,
                         ui::IconButtons::kSegments,
                         hovered ? ui::IconButtons::kButtonFillHover
                                 : ui::IconButtons::kButtonFill);
    DrawRectangleRoundedLinesEx(buttonRect, ui::IconButtons::kRoundness,
                                ui::IconButtons::kSegments,
                                ui::IconButtons::kBorderWidth,
                                hovered ? ui::IconButtons::kButtonBorderHover
                                        : ui::IconButtons::kButtonBorder);

    int fontSize = static_cast<int>(20 * scale);
    DrawText(text, 
             static_cast<int>(buttonRect.x + (buttonRect.width - MeasureText(text, fontSize)) / 2), 
             static_cast<int>(buttonRect.y + (buttonRect.height - fontSize) / 2), 
             fontSize, ui::Dialog::kTextPrimary);
}

void MenuView::render(const MenuModel& model) {
    // Match background color with the game theme
    ClearBackground({26, 31, 41, 255});

    ui::AutoLayout::Metrics metrics = ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
    float scale = metrics.uiScale;
    
    float centerX = GetScreenWidth() / 2.0f;
    float centerY = GetScreenHeight() / 2.0f;

    float btnWidth = 240.0f * scale;
    float btnHeight = 56.0f * scale;
    float btnGap = 80.0f * scale;

    int titleFont = static_cast<int>(40 * scale);
    int subtitleFont = static_cast<int>(20 * scale);

    DrawText("KING CHESS", static_cast<int>(centerX - MeasureText("KING CHESS", titleFont) / 2), static_cast<int>(centerY - 150 * scale), titleFont, ui::Dialog::kTextPrimary);

    if (model.state == MenuState::MAIN) {
        Rectangle b0 = makeButton(centerX, centerY, 0, btnWidth, btnHeight, btnGap);
        Rectangle b1 = makeButton(centerX, centerY, 1, btnWidth, btnHeight, btnGap);
        Vector2 m = GetMousePosition();
        bool h0 = CheckCollisionPointRec(m, b0);
        bool h1 = CheckCollisionPointRec(m, b1);
        
        drawMenuButton(b0, h0, "Player vs Player", scale);
        drawMenuButton(b1, h1, "Player vs Bot", scale);
    } else {
        const char* modeText = (model.selectedMode == PlayMode::PvP) ? "Mode: PvP" : "Mode: PvE";
        DrawText(modeText, static_cast<int>(centerX - MeasureText(modeText, subtitleFont) / 2), static_cast<int>(centerY - 100 * scale), subtitleFont, ui::Dialog::kTextPrimary);

        Rectangle n = makeButton(centerX, centerY, 0, btnWidth, btnHeight, btnGap);
        Rectangle l = makeButton(centerX, centerY, 1, btnWidth, btnHeight, btnGap);
        Rectangle back = {centerX - btnWidth / 2.0f, centerY + 100.0f * scale, btnWidth, btnHeight};
        Vector2 m = GetMousePosition();

        bool hn = CheckCollisionPointRec(m, n);
        bool hl = CheckCollisionPointRec(m, l);
        bool hb = CheckCollisionPointRec(m, back);

        drawMenuButton(n, hn, "New Game", scale);
        drawMenuButton(l, hl, "Load Profile", scale);
        drawMenuButton(back, hb, "Back", scale);
    }
}
