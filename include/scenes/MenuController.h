#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include "scenes/MenuModel.h"
#include "scenes/MenuView.h"
#include "scenes/SceneManager.h"

class MenuController {
public:
    MenuController(MenuModel* model, MenuView* view);
    void update(SceneManager* manager);

private:
    MenuModel* model_;
    MenuView* view_;
};

#endif // MENUCONTROLLER_H
