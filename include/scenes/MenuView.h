#ifndef MENUVIEW_H
#define MENUVIEW_H

#include <raylib.h>
#include "scenes/MenuModel.h"

class MenuView {
public:
    MenuView();
    ~MenuView();

    // Render the menu based on model state
    void render(const MenuModel& model);

private:
    Texture2D titleImage_;
    Texture2D bgImage_;
};

#endif // MENUVIEW_H
