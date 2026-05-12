#ifndef APP_H
#define APP_H

#include <raylib.h>
#include <memory>
#include "core/SceneManager.h"
#include "core/Scene.h"

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
    // Interface của SceneManager
    void changeScene(SceneType type) override;
    void quit() override;

    void run();
};

#endif // APP_H
