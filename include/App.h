#ifndef APP_H
#define APP_H

#include <raylib.h>
#include <memory>
#include "scenes/SceneManager.h"
#include "scenes/Scene.h"

class App : public SceneManager {
private:
    std::unique_ptr<Scene> currentScene_;
    SceneType pendingSceneType_;
    bool isSceneChangePending_ = false;
    bool shouldClose_ = false;

    // Hàm tạo Scene theo enum, giải quyết Circular Dependency
    std::unique_ptr<Scene> createScene(SceneType type);

public:
    App();
    ~App() override = default;

    // Interface của SceneManager
    void changeScene(SceneType type) override {
        // Đặt cờ để đổi cảnh vào cuối frame, tránh xoá vùng nhớ khi đang chạy update
        pendingSceneType_ = type;
        isSceneChangePending_ = true;
    }

    void quit() override {
        shouldClose_ = true;
    }

    void run() {
        while (!WindowShouldClose() && !shouldClose_) {
            // Đổi cảnh nếu có yêu cầu
            if (isSceneChangePending_) {
                currentScene_ = createScene(pendingSceneType_);
                isSceneChangePending_ = false;
            }

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
