#ifndef APP_H
#define APP_H

#include <raylib.h>
#include "scenes/Scene.h"

class App {
private:
    Scene* currentScene_ = nullptr;
    bool shouldClose_ = false;

public:
    App(Scene* initialScene) : currentScene_(initialScene) {}
    
    ~App() {
        delete currentScene_;
    }

    void changeScene(Scene* newScene) {
        delete currentScene_;
        currentScene_ = newScene;
    }

    void quit() {
        shouldClose_ = true;
    }

    void run() {
        while (!WindowShouldClose() && !shouldClose_) {
            // 1. Update logic của Scene hiện tại
            if (currentScene_) {
                currentScene_->update(this);
            }

            // 2. Render Scene hiện tại
            BeginDrawing();
            ClearBackground(RAYWHITE);
            if (currentScene_) {
                currentScene_->render();
            }
            EndDrawing();
        }
    }
};

#endif // APP_H
