#ifndef MENUMODEL_H
#define MENUMODEL_H

enum class MenuState {
    MAIN,
    SELECT_MODE
};

enum class PlayMode {
    PvP,
    PvE
};

class MenuModel {
private:
    MenuState state_ = MenuState::MAIN;
    PlayMode selectedMode_ = PlayMode::PvP;

public:
    MenuState getState() const { return state_; }
    void setState(MenuState s) { state_ = s; }

    PlayMode getSelectedMode() const { return selectedMode_; }
    void setSelectedMode(PlayMode m) { selectedMode_ = m; }
};

#endif // MENUMODEL_H
