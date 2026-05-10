#ifndef PLAYINGSCENE_H
#define PLAYINGSCENE_H

#include "scenes/Scene.h"
#include "scenes/MainMenuScene.h" // For PlayMode
#include "scenes/SceneManager.h"
#include <memory>
#include "Game.h"
#include "ChessView.h"
#include "ChessController.h"
#include "ChessSound.h"

class PlayingScene : public Scene {
private:
    std::unique_ptr<Game> game_;
    std::unique_ptr<ChessView> view_;
    std::unique_ptr<ChessSound> sound_;
    std::unique_ptr<ChessController> controller_;
    PlayMode mode_;
    bool shouldLoadSave_ = false;
    bool initialized_ = false;
    bool loadFailed_ = false;
    double loadFailStartTime_ = 0.0;

    static constexpr double kLoadFailMessageDurationSeconds_ = 1.5;
    static constexpr const char* kLoadFailMessage_ =
        "Failed to load save.bin - returning to menu";

public:
    PlayingScene(PlayMode mode, bool loadSave);
    ~PlayingScene() override;

    void update(SceneManager* manager) override;
    void render() override;
};

#endif // PLAYINGSCENE_H
