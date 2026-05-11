#ifndef SCENE_H
#define SCENE_H

class SceneManager; // Forward declaration

class Scene {
public:
    virtual ~Scene() = default;
    
    // Truyền SceneManager vào để Scene có thể gọi đổi cảnh
    virtual void update(SceneManager* manager) = 0; 
    
    virtual void render() = 0;         
};

#endif // SCENE_H
