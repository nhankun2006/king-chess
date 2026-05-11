#ifndef MENUVIEW_H
#define MENUVIEW_H

#include <raylib.h>
#include "scenes/main_menu/MenuModel.h"

class MenuView {
public:
    MenuView() = default;

    // Render the menu based on model state
    void render(const MenuModel& model);
};

#endif // MENUVIEW_H
