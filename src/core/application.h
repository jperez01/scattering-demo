#pragma once

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <renderer/gl_renderer.h>

class Application {
public:
    Application();
	void init();
    void cleanup();
    void mainLoop();

private:
    void handleEvents();
    void handleImportedObjs();

    void handleMouse(double xposIn, double yposIn);
    void handleScroll(double yoffset);
    void handleSizeChange(int width, int height);

    void handleClick(double xposIn, double yposIn);
    void checkIntersection(glm::vec4& origin, glm::vec4& direction, glm::vec4& inverse_dir);

    void asyncLoadModel(std::string path, FileType type = OBJ);

	GLRenderer mRenderer;
    SceneEditor mEditor;

    SDL_Window* window{};
    SDL_GLContext gl_context{};
    int WINDOW_WIDTH = 1920, WINDOW_HEIGHT = 1080;

    bool closedWindow = false;
    bool keyDown[4] = { false, false, false, false };

    float lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
    bool firstMouse = true;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    std::vector<Model> importedObjs;
    std::vector<Model> usableObjs;
    int chosenObjIndex = 0;

    efsw::FileWatcher fileWatcher;
    UpdateListener updateListener;

    Camera camera;
    bool handleMouseMovement = true;
};