#include "scenes/main_menu/MenuView.h"
#include "scenes/PlayMode.h"
#include "scenes/main_menu/MainMenuScene.h"
#include "config/UIConfig.h"
#include "ui/Button.h"
#include "ui/Dialog.h"
#include <string>

MenuView::MenuView() {
    titleImage_ = LoadTexture("assets/images/game_title.png");
    bgImage_ = LoadTexture("assets/images/menu_background.png");
}

MenuView::~MenuView() {
    UnloadTexture(titleImage_);
    UnloadTexture(bgImage_);
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

    auto makeButtonBounds = [&](int idx) -> Rectangle {
        float y = centerY + idx * btnGap - btnHeight * 0.8f + 50.0f * (btnHeight / 56.0f);
        return {centerX - btnWidth / 2.0f, y, btnWidth, btnHeight};
    };

    auto drawBtn = [&](Rectangle bounds, const char* text) {
        ui::ButtonStyle style;
        style.fontSize = static_cast<int>(26 * scale);
        style.baseColor = ui::IconButtons::kButtonFill;
        style.hoverColor = ui::IconButtons::kButtonFillHover;
        style.borderColor = ui::IconButtons::kButtonBorder;
        style.borderHoverColor = ui::IconButtons::kButtonBorderHover;
        style.textColor = ui::Dialog::kTextPrimary;
        style.roundness = ui::IconButtons::kRoundness;
        style.segments = ui::IconButtons::kSegments;
        style.borderWidth = ui::IconButtons::kBorderWidth;

        ui::Button btn(bounds, text, style);
        btn.update(GetMousePosition());
        btn.draw();
    };

    auto drawBackBtn = [&](Rectangle bounds) {
        ui::ButtonStyle style;
        style.baseColor = ui::IconButtons::kButtonFill;
        style.hoverColor = ui::IconButtons::kButtonFillHover;
        style.borderColor = ui::IconButtons::kButtonBorder;
        style.borderHoverColor = ui::IconButtons::kButtonBorderHover;
        style.textColor = ui::Dialog::kTextPrimary;
        style.fontSize = static_cast<int>(20 * scale);

        ui::Button btn(bounds, "<-", style);
        btn.update(GetMousePosition());
        btn.draw();
    };

    if (titleImage_.id != 0) {
        float titleScale = scale * 0.5f;
        float imgWidth = titleImage_.width * titleScale;
        float imgHeight = titleImage_.height * titleScale;
        Rectangle dest = { centerX - imgWidth / 2.0f, centerY - 150.0f * scale - imgHeight / 2.0f, imgWidth, imgHeight };
        DrawTexturePro(titleImage_, {0, 0, (float)titleImage_.width, (float)titleImage_.height}, dest, {0, 0}, 0.0f, WHITE);
    } else {
        DrawText("KING CHESS", static_cast<int>(centerX - MeasureText("KING CHESS", titleFont) / 2), static_cast<int>(centerY - 150 * scale), titleFont, ui::Dialog::kTextPrimary);
    }

    Rectangle backBounds = { 20.0f * scale, GetScreenHeight() - 40.0f * scale - 20.0f * scale, 80.0f * scale, 40.0f * scale };

    if (model.state == MenuState::HOME) {
        drawBtn(makeButtonBounds(0), "Play");
        drawBtn(makeButtonBounds(1), "Settings");
        drawBtn(makeButtonBounds(2), "Quit");
        drawBtn({ GetScreenWidth() - 150.0f * scale, GetScreenHeight() - 60.0f * scale, 130.0f * scale, 40.0f * scale }, "Credit");
    } else if (model.state == MenuState::PLAY_SELECT) {
        drawBtn(makeButtonBounds(0), "Player vs Player");
        drawBtn(makeButtonBounds(1), "Player vs Bot");
        drawBackBtn(backBounds);
    } else if (model.state == MenuState::SELECT_MODE) {
        drawBtn(makeButtonBounds(0), "New Game");
        drawBtn(makeButtonBounds(1), "Load Profile");
        drawBackBtn(backBounds);
    } else if (model.state == MenuState::SELECT_TIMER) {
        float arrowSize = 60.0f * scale;
        float boxWidth = 200.0f * scale;
        
        Rectangle leftArrowBtn = { centerX - boxWidth/2 - arrowSize - 20.0f*scale, centerY - arrowSize/2 + 50.0f * (btnHeight / 56.0f), arrowSize, arrowSize };
        Rectangle rightArrowBtn = { centerX + boxWidth/2 + 20.0f*scale, centerY - arrowSize/2 + 50.0f * (btnHeight / 56.0f), arrowSize, arrowSize };
        Rectangle startBtn = makeButtonBounds(1);
        startBtn.y += 0.5f * btnGap; // Adjusted for 1.5f index in original

        ui::ButtonStyle iconStyle;
        iconStyle.fontSize = static_cast<int>(40 * scale);
        iconStyle.baseColor = ui::IconButtons::kButtonFill;
        iconStyle.hoverColor = ui::IconButtons::kButtonFillHover;
        iconStyle.borderColor = ui::IconButtons::kButtonBorder;
        iconStyle.borderHoverColor = ui::IconButtons::kButtonBorderHover;
        
        ui::Button btnLeft(leftArrowBtn, "<", iconStyle);
        btnLeft.update(GetMousePosition());
        btnLeft.draw();

        ui::Button btnRight(rightArrowBtn, ">", iconStyle);
        btnRight.update(GetMousePosition());
        btnRight.draw();

        Rectangle timeBox = { centerX - boxWidth/2, centerY - arrowSize/2 + 50.0f * (btnHeight / 56.0f), boxWidth, arrowSize };
        
        std::string timeStr = std::to_string(model.selectedTimer) + " Min";
        int tSize = static_cast<int>(30*scale);
        DrawText(timeStr.c_str(), static_cast<int>(timeBox.x + (timeBox.width - MeasureText(timeStr.c_str(), tSize))/2), static_cast<int>(timeBox.y + (timeBox.height - tSize)/2), tSize, ui::Dialog::kTextPrimary);

        drawBtn(startBtn, "Start Game");
        drawBackBtn(backBounds);
    } else if (model.state == MenuState::SETTINGS) {
        const char* text = "Settings... Wait for next update!";
        DrawText(text, static_cast<int>(centerX - MeasureText(text, subtitleFont) / 2), static_cast<int>(centerY), subtitleFont, ui::Dialog::kTextPrimary);
        drawBackBtn(backBounds);
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
        drawBackBtn(backBounds);
    }
}
