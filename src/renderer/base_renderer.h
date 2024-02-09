#pragma once

#include <vector>

#include "shader/update_listener.h"
#include "shader/shader.h"
#include "utils/camera.h"
#include "assets/model.h"
#include "utils/common_primitives.h"

#include "ui/editor.h"

enum DrawOptions {
    SKIP_TEXTURES = (1u << 0),
    SKIP_CULLING = (1u << 1)
};

class BaseRenderer {
public:
    virtual ~BaseRenderer() = default;

    virtual void init_resources();
    virtual void handleObjs(std::vector<Model>& objs);
    void loadModelData(Model& model);

    virtual void render(std::vector<Model>& objs) = 0;
    virtual void handleImGui() = 0;

    virtual void subscribePrograms(UpdateListener& listener);

    Camera* camera = nullptr;
    int WINDOW_WIDTH = 1920, WINDOW_HEIGHT = 1080;
    glm::ivec2 windowSize = glm::ivec2(WINDOW_WIDTH, WINDOW_HEIGHT);

    ScreenQuad screenQuad;
    EnviornmentCubemap cubemap;

protected:
    float startTime = 0.0f;
    float animationTime = 0.0f;
    int chosenAnimation = 0;

    void drawModels(std::vector<Model>& models, Shader& shader, unsigned char drawOptions = 0) const;
    void checkFrustum(std::vector<Model>& objs) const;
};