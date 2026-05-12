#ifndef MENUMODEL_H
#define MENUMODEL_H

// Forward-declare enums to avoid circular includes
enum class PlayMode;
enum class MenuState;

class MenuModel {
public:
    MenuModel();

    PlayMode selectedMode;
    MenuState state;
    int selectedTimer = 5;
};

extern int g_targetTimeControl;

#endif // MENUMODEL_H
