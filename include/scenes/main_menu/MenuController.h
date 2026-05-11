#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include "scenes/main_menu/MenuModel.h"
#include "scenes/main_menu/MenuView.h"
#include "core/SceneManager.h"

class MenuController {
public:
    MenuController(MenuModel* model, MenuView* view);
    void update(SceneManager* manager);

private:
    MenuModel* model_;
    MenuView* view_;
};

#endif // MENUCONTROLLER_H
