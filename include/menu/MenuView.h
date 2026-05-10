#ifndef MENUVIEW_H
#define MENUVIEW_H

#include <raylib.h>
#include "MenuModel.h"

class MenuView {
public:
    Rectangle getPvpBtnRect() const;
    Rectangle getPveBtnRect() const;
    Rectangle getNewGameBtnRect() const;
    Rectangle getLoadGameBtnRect() const;
    Rectangle getBackBtnRect() const;

    void render(const MenuModel& model) const;
};

#endif // MENUVIEW_H
