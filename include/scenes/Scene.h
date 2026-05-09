#ifndef SCENE_H
#define SCENE_H

class App; // Forward declaration

class Scene {
public:
    virtual ~Scene() = default;
    
    // Xử lý input và logic của scene (gọi mỗi frame)
    virtual void update(App* app) = 0; 
    
    // Vẽ UI bằng raylib (gọi mỗi frame, đã được bọc trong BeginDrawing/EndDrawing)
    virtual void render() = 0;         
};

#endif // SCENE_H
