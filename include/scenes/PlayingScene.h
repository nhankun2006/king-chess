#ifndef PLAYINGSCENE_H
#define PLAYINGSCENE_H

#include "scenes/Scene.h"
#include "scenes/MainMenuScene.h" // For PlayMode
#include "Game.h"
#include "ChessView.h"
#include "ChessControllder.h"

class PlayingScene : public Scene {
private:
    Game* game_;
    ChessView* view_;
    ChessController* controller_;
    PlayMode mode_;
    bool isLoaded_;

public:
    PlayingScene(PlayMode mode, bool loadSave);
    ~PlayingScene() override;

    void update(App* app) override;
    void render() override;
};

#endif // PLAYINGSCENE_H
