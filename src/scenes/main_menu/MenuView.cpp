#include "scenes/main_menu/MenuView.h"
#include "scenes/PlayMode.h"
#include "scenes/main_menu/MainMenuScene.h"
#include "config/UIConfig.h"
#include <string>

MenuView::MenuView() {
    titleImage_ = LoadTexture("assets/images/game_title.png");
    bgImage_ = LoadTexture("assets/images/menu_background.png");
}

MenuView::~MenuView() {
    UnloadTexture(titleImage_);
    UnloadTexture(bgImage_);
}

static Rectangle makeButton(float centerX, float centerY, int idx, float btnWidth, float btnHeight, float btnGap) {
    float y = centerY + idx * btnGap - btnHeight * 0.8f + 50.0f * (btnHeight / 56.0f);
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

    int fontSize = static_cast<int>(26 * scale);
    int textX = static_cast<int>(buttonRect.x + (buttonRect.width - MeasureText(text, fontSize)) / 2);
    int textY = static_cast<int>(buttonRect.y + (buttonRect.height - fontSize) / 2);
    
    // Draw text twice with an offset to simulate bold weight
    DrawText(text, textX, textY, fontSize, ui::Dialog::kTextPrimary);
    DrawText(text, textX + 1, textY, fontSize, ui::Dialog::kTextPrimary);
    DrawText(text, textX, textY + 1, fontSize, ui::Dialog::kTextPrimary);
    DrawText(text, textX + 1, textY + 1, fontSize, ui::Dialog::kTextPrimary);
}

static void drawBackButton(Rectangle buttonRect, bool hovered, float scale) {
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

    // Draw left arrow
    Vector2 center = { buttonRect.x + buttonRect.width / 2.0f, buttonRect.y + buttonRect.height / 2.0f };
    float size = 15.0f * scale;
    float thick = 3.0f * scale;
    Color color = ui::Dialog::kTextPrimary;
    
    DrawLineEx({ center.x + size * 0.5f, center.y }, { center.x - size * 0.5f, center.y }, thick, color);
    DrawLineEx({ center.x - size * 0.5f, center.y }, { center.x, center.y - size * 0.5f }, thick, color);
    DrawLineEx({ center.x - size * 0.5f, center.y }, { center.x, center.y + size * 0.5f }, thick, color);
}

void MenuView::render(const MenuModel& model) {
    // Match background color with the game theme
    ClearBackground({26, 31, 41, 255});

    if (bgImage_.id != 0) {
        Rectangle source = { 0.0f, 0.0f, (float)bgImage_.width, (float)bgImage_.height };
        float screenAspectRatio = (float)GetScreenWidth() / (float)GetScreenHeight();
        float bgAspectRatio = (float)bgImage_.width / (float)bgImage_.height;
        Rectangle dest;
        if (screenAspectRatio > bgAspectRatio) {
            float width = GetScreenWidth();
            float height = width / bgAspectRatio;
            dest = { 0.0f, (GetScreenHeight() - height) / 2.0f, width, height };
        } else {
            float height = GetScreenHeight();
            float width = height * bgAspectRatio;
            dest = { (GetScreenWidth() - width) / 2.0f, 0.0f, width, height };
        }
        DrawTexturePro(bgImage_, source, dest, {0.0f, 0.0f}, 0.0f, WHITE);
    }

    ui::AutoLayout::Metrics metrics = ui::AutoLayout::ComputeMetrics(GetScreenWidth(), GetScreenHeight());
    float scale = metrics.uiScale;
    
    float centerX = GetScreenWidth() / 2.0f;
    float centerY = GetScreenHeight() / 2.0f;

    float btnWidth = 240.0f * scale;
    float btnHeight = 56.0f * scale;
    float btnGap = 80.0f * scale;

    int titleFont = static_cast<int>(40 * scale);
    int subtitleFont = static_cast<int>(20 * scale);

    if (titleImage_.id != 0) {
        float titleScale = scale * 0.5f;
        float imgWidth = titleImage_.width * titleScale;
        float imgHeight = titleImage_.height * titleScale;
        Rectangle dest = { centerX - imgWidth / 2.0f, centerY - 150.0f * scale - imgHeight / 2.0f, imgWidth, imgHeight };
        DrawTexturePro(titleImage_, {0, 0, (float)titleImage_.width, (float)titleImage_.height}, dest, {0, 0}, 0.0f, WHITE);
    } else {
        DrawText("KING CHESS", static_cast<int>(centerX - MeasureText("KING CHESS", titleFont) / 2), static_cast<int>(centerY - 150 * scale), titleFont, ui::Dialog::kTextPrimary);
    }

    if (model.state == MenuState::HOME) {
        Rectangle b0 = makeButton(centerX, centerY, 0, btnWidth, btnHeight, btnGap);
        Rectangle b1 = makeButton(centerX, centerY, 1, btnWidth, btnHeight, btnGap);
        Rectangle b2 = makeButton(centerX, centerY, 2, btnWidth, btnHeight, btnGap);
        Rectangle creditBtn = { GetScreenWidth() - 150.0f * scale, GetScreenHeight() - 60.0f * scale, 130.0f * scale, 40.0f * scale };
        Vector2 m = GetMousePosition();

        drawMenuButton(b0, CheckCollisionPointRec(m, b0), "Play", scale);
        drawMenuButton(b1, CheckCollisionPointRec(m, b1), "Settings", scale);
        drawMenuButton(b2, CheckCollisionPointRec(m, b2), "Quit", scale);
        drawMenuButton(creditBtn, CheckCollisionPointRec(m, creditBtn), "Credit", scale);
    } else if (model.state == MenuState::PLAY_SELECT) {
        Rectangle b0 = makeButton(centerX, centerY, 0, btnWidth, btnHeight, btnGap);
        Rectangle b1 = makeButton(centerX, centerY, 1, btnWidth, btnHeight, btnGap);
        Rectangle back = { 20.0f * scale, GetScreenHeight() - 40.0f * scale - 20.0f * scale, 80.0f * scale, 40.0f * scale };
        Vector2 m = GetMousePosition();
        
        drawMenuButton(b0, CheckCollisionPointRec(m, b0), "Player vs Player", scale);
        drawMenuButton(b1, CheckCollisionPointRec(m, b1), "Player vs Bot", scale);
        drawBackButton(back, CheckCollisionPointRec(m, back), scale);
    } else if (model.state == MenuState::SELECT_MODE) {
        Rectangle n = makeButton(centerX, centerY, 0, btnWidth, btnHeight, btnGap);
        Rectangle l = makeButton(centerX, centerY, 1, btnWidth, btnHeight, btnGap);
        Rectangle back = { 20.0f * scale, GetScreenHeight() - 40.0f * scale - 20.0f * scale, 80.0f * scale, 40.0f * scale };
        Vector2 m = GetMousePosition();

        drawMenuButton(n, CheckCollisionPointRec(m, n), "New Game", scale);
        drawMenuButton(l, CheckCollisionPointRec(m, l), "Load Profile", scale);
        drawBackButton(back, CheckCollisionPointRec(m, back), scale);
    } else if (model.state == MenuState::SELECT_TIMER) {
        // Time selection with left/right arrows
        float arrowSize = 60.0f * scale; // Big arrows
        float boxWidth = 200.0f * scale;
        
        Rectangle leftArrowBtn = { centerX - boxWidth/2 - arrowSize - 20.0f*scale, centerY - arrowSize/2 + 50.0f * (btnHeight / 56.0f), arrowSize, arrowSize };
        Rectangle rightArrowBtn = { centerX + boxWidth/2 + 20.0f*scale, centerY - arrowSize/2 + 50.0f * (btnHeight / 56.0f), arrowSize, arrowSize };
        Rectangle startBtn = makeButton(centerX, centerY, 1.5f, btnWidth, btnHeight, btnGap);
        Rectangle back = { 20.0f * scale, GetScreenHeight() - 40.0f * scale - 20.0f * scale, 80.0f * scale, 40.0f * scale };
        Vector2 m = GetMousePosition();

        // Draw left arrow
        bool leftHover = CheckCollisionPointRec(m, leftArrowBtn);
        DrawRectangleRounded(leftArrowBtn, ui::IconButtons::kRoundness, ui::IconButtons::kSegments, leftHover ? ui::IconButtons::kButtonFillHover : ui::IconButtons::kButtonFill);
        DrawRectangleRoundedLinesEx(leftArrowBtn, ui::IconButtons::kRoundness, ui::IconButtons::kSegments, ui::IconButtons::kBorderWidth, leftHover ? ui::IconButtons::kButtonBorderHover : ui::IconButtons::kButtonBorder);
        DrawText("<", static_cast<int>(leftArrowBtn.x + leftArrowBtn.width/2 - MeasureText("<", static_cast<int>(40*scale))/2), static_cast<int>(leftArrowBtn.y + leftArrowBtn.height/2 - 20*scale), static_cast<int>(40*scale), ui::Dialog::kTextPrimary);

        // Draw right arrow
        bool rightHover = CheckCollisionPointRec(m, rightArrowBtn);
        DrawRectangleRounded(rightArrowBtn, ui::IconButtons::kRoundness, ui::IconButtons::kSegments, rightHover ? ui::IconButtons::kButtonFillHover : ui::IconButtons::kButtonFill);
        DrawRectangleRoundedLinesEx(rightArrowBtn, ui::IconButtons::kRoundness, ui::IconButtons::kSegments, ui::IconButtons::kBorderWidth, rightHover ? ui::IconButtons::kButtonBorderHover : ui::IconButtons::kButtonBorder);
        DrawText(">", static_cast<int>(rightArrowBtn.x + rightArrowBtn.width/2 - MeasureText(">", static_cast<int>(40*scale))/2), static_cast<int>(rightArrowBtn.y + rightArrowBtn.height/2 - 20*scale), static_cast<int>(40*scale), ui::Dialog::kTextPrimary);

        // Draw current time box
        Rectangle timeBox = { centerX - boxWidth/2, centerY - arrowSize/2 + 50.0f * (btnHeight / 56.0f), boxWidth, arrowSize };
        
        std::string timeStr = std::to_string(model.selectedTimer) + " Min";
        int tSize = static_cast<int>(30*scale);
        DrawText(timeStr.c_str(), static_cast<int>(timeBox.x + (timeBox.width - MeasureText(timeStr.c_str(), tSize))/2), static_cast<int>(timeBox.y + (timeBox.height - tSize)/2), tSize, ui::Dialog::kTextPrimary);

        drawMenuButton(startBtn, CheckCollisionPointRec(m, startBtn), "Start Game", scale);
        drawBackButton(back, CheckCollisionPointRec(m, back), scale);
    } else if (model.state == MenuState::SETTINGS) {
        const char* text = "Settings... Wait for next update!";
        DrawText(text, static_cast<int>(centerX - MeasureText(text, subtitleFont) / 2), static_cast<int>(centerY), subtitleFont, ui::Dialog::kTextPrimary);
        
        Rectangle back = { 20.0f * scale, GetScreenHeight() - 40.0f * scale - 20.0f * scale, 80.0f * scale, 40.0f * scale };
        drawBackButton(back, CheckCollisionPointRec(GetMousePosition(), back), scale);
    } else if (model.state == MenuState::CREDITS) {
        int yOffset = -50;
        const char* credits[] = {
            "KING CHESS",
            "Developed by King Chess Team",
            "Thank you for playing!"
        };
        for (const char* line : credits) {
            DrawText(line, static_cast<int>(centerX - MeasureText(line, subtitleFont) / 2), static_cast<int>(centerY + yOffset * scale), subtitleFont, ui::Dialog::kTextPrimary);
            yOffset += 40;
        }

        Rectangle back = { 20.0f * scale, GetScreenHeight() - 40.0f * scale - 20.0f * scale, 80.0f * scale, 40.0f * scale };
        drawBackButton(back, CheckCollisionPointRec(GetMousePosition(), back), scale);
    }
}
