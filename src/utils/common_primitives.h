#pragma once

#include "shader/shader.h"
#include "utils/functions.h"

namespace glutil {
    AllocatedBuffer createUnitCube();
    AllocatedBuffer createScreenQuad();
    AllocatedBuffer createPlane();
};

struct EnviornmentCubemap {
    AllocatedBuffer buffer{};
    unsigned int texture{};
    Shader pipeline;

    EnviornmentCubemap() = default;

    explicit EnviornmentCubemap(const std::string& path) {
        pipeline = Shader("cubemap/map.vs", "cubemap/map.fs");

        buffer = glutil::createUnitCube();
        texture = glutil::loadCubemap(path);
    }

    void draw(const glm::mat4& projection, const glm::mat4& view) const {
        const glm::mat4 convertedView = glm::mat4(glm::mat3(view));

        glDepthFunc(GL_LEQUAL);
        pipeline.use();
        pipeline.setMat4("projection", projection);
        pipeline.setMat4("view", convertedView);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, texture);
        pipeline.setInt("skybox", 0);

        glBindVertexArray(buffer.VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);
    }
};

struct ScreenQuad {
    AllocatedBuffer buffer{};

    ScreenQuad() = default;

    void init() {
        buffer = glutil::createScreenQuad();
    }

    void draw() const {
        glBindVertexArray(buffer.VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
};