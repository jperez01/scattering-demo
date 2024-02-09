#pragma once

#include "base_renderer.h"

class GLRenderer : public BaseRenderer {
public:
    void init_resources() override;
    void subscribePrograms(UpdateListener& listener) override;
    void render(std::vector<Model>& objs) override;
    void handleImGui() override;

private:
    AllocatedBuffer planeBuffer;
    unsigned int planeTexture;

    Shader starterPipeline;

    void renderScene(std::vector<Model>& objs, Shader& shader, bool skipTextures);
};