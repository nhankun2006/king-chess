#ifndef MENUCONTROLLER_H
#define MENUCONTROLLER_H

#include "MenuModel.h"
#include "MenuView.h"
#include "scenes/SceneManager.h"

class MenuController {
private:
    MenuModel* model_;
    MenuView* view_;

public:
    MenuController(MenuModel* model, MenuView* view) 
        : model_(model), view_(view) {}
    
    // Xử lý logic input và yêu cầu chuyển cảnh nếu cần
    void processInput(SceneManager* manager);
};

#endif // MENUCONTROLLER_H
