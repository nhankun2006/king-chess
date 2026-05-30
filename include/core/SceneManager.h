#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <memory>

class Scene;

enum class SceneType {
    MAIN_MENU,
    PLAYING_PVP,
    PLAYING_PVE,
    PLAYING_LOADED_PVP,
    PLAYING_LOADED_PVE,
    LOBBY
};

class SceneManager {
public:
    virtual ~SceneManager() = default;
    
    // Giao diện để các scene yêu cầu chuyển cảnh hoặc thoát game
    virtual void changeScene(SceneType type) = 0;
    virtual void changeSceneWithInstance(std::unique_ptr<Scene> scene) = 0;
    virtual void quit() = 0;
};

#endif // SCENEMANAGER_H
