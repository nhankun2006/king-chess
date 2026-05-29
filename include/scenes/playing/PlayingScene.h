#ifndef PLAYINGSCENE_H
#define PLAYINGSCENE_H

#include "core/Scene.h"
#include "scenes/PlayMode.h"
#include "core/SceneManager.h"
#include <memory>
#include <string>
#include "chess/model/Game.h"
#include "scenes/playing/ChessView.h"
#include "scenes/playing/ChessController.h"
#include "services/ChessSound.h"

class NetworkSession;

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
    std::string loadFailMessage_;

    static constexpr double kLoadFailMessageDurationSeconds_ = 1.5;

    // Network session (owned by the LobbyScene, shared via raw pointer)
    NetworkSession *networkSession_ = nullptr;

public:
    PlayingScene(PlayMode mode, bool loadSave,
                 NetworkSession *networkSession = nullptr);
    ~PlayingScene() override;

    void update(SceneManager* manager) override;
    void render() override;
};

#endif // PLAYINGSCENE_H

