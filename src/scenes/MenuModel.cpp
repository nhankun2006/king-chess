
#include "scenes/MenuModel.h"
#include "scenes/MainMenuScene.h"

MenuModel::MenuModel()
	: selectedMode(PlayMode::PvP), state(MenuState::HOME) {}

int g_targetTimeControl = 5;

