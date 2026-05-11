#include "scenes/PlayingScene.h"
#include "ChessSound.h"
#include "scenes/MenuModel.h"
#include <memory>

PlayingScene::PlayingScene(PlayMode mode, bool loadSave)
    : game_(nullptr), view_(nullptr), controller_(nullptr), mode_(mode), isLoaded_(loadSave) {}

PlayingScene::~PlayingScene() {
    // Ensure game is saved when the playing scene is destroyed (exit or scene switch)
    if (game_ != nullptr) {
        std::string saveFile = (mode_ == PlayMode::PvP) ? "save_pvp.bin" : "save_pve.bin";
        game_->saveGame(saveFile);
    }
    delete controller_;
    delete view_;
    delete game_;
}

void PlayingScene::update(SceneManager* manager) {
    // If not started, initialize game and run the blocking controller loop
    if (game_ == nullptr) {
        std::string saveFile = (mode_ == PlayMode::PvP) ? "save_pvp.bin" : "save_pve.bin";
        
        game_ = new Game();
        game_->setTimeControl(g_targetTimeControl);
        view_ = new ChessView();
        ChessSound* sound = new ChessSound();
        sound->loadSounds();
        game_->attach(view_);
        game_->attach(sound);

        if (!view_->LoadAssets()) {
            // Failed to load assets: return to menu
            delete sound;
            manager->changeScene(SceneType::MAIN_MENU);
            return;
        }

        controller_ = new ChessController(*game_, *view_, saveFile);

        // If load flag set, attempt to load save (Game::loadFromFile assumed)
        if (isLoaded_) {
            // Use Game's load mechanism
            bool ok = game_->loadGame(saveFile);
            if (!ok) {
                // Show a brief error message and return to main menu
                std::string msgStr = "Failed to load " + saveFile + " - returning to menu";
                const char* msg = msgStr.c_str();
                const double start = GetTime();
                while (GetTime() - start < 1.5 && !WindowShouldClose()) {
                    BeginDrawing();
                    ClearBackground({30, 30, 30, 255});
                    DrawText(msg, GetScreenWidth() / 2 - MeasureText(msg, 20) / 2,
                             GetScreenHeight() / 2 - 10, 20, RAYWHITE);
                    EndDrawing();
                }
                manager->changeScene(SceneType::MAIN_MENU);
                return;
            }
        }

        // Run the game's own loop (blocking). When it returns, go back to main menu.
        controller_->run();

        if (WindowShouldClose()) {
            manager->quit();
        } else {
            manager->changeScene(SceneType::MAIN_MENU);
        }
    }
}

void PlayingScene::render() {
    // Optionally show a loading message before the controller starts
    if (game_ == nullptr) {
        ClearBackground(BLACK);
        const char* txt = "Starting game...";
        DrawText(txt, GetScreenWidth()/2 - MeasureText(txt, 20)/2, GetScreenHeight()/2 - 10, 20, WHITE);
    }
}
