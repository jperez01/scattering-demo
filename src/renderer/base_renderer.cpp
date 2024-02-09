#include "base_renderer.h"
#include "utils/functions.h"
#include "stb_image.h"

#include <SDL.h>
#include <thread>
#include <future>
#include <glm/gtc/matrix_transform.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_stdlib.h"
#include "ImGuizmo/ImGuizmo.h"

void BaseRenderer::init_resources() {
    startTime = static_cast<float>(SDL_GetTicks());
}

void BaseRenderer::subscribePrograms(UpdateListener&listener) {}
void BaseRenderer::handleObjs(std::vector<Model>& objs) {}

void BaseRenderer::drawModels(std::vector<Model>& models, Shader& shader, unsigned char drawOptions) const {
    bool shouldSkipTextures = drawOptions & SKIP_TEXTURES;
    bool shouldSkipCulling = drawOptions & SKIP_CULLING;

    for (Model& model : models) {
        if (!shouldSkipCulling) {
            glm::vec4 transformedMax = model.model_matrix * model.aabb.maxPoint;
            glm::vec4 transformedMin = model.model_matrix * model.aabb.minPoint;
            bool shouldDraw = camera->isInsideFrustum(transformedMax, transformedMin);
            if (!shouldDraw) continue;
        }

        for (int j = 0; j < model.meshes.size(); j++) {
            Mesh& mesh = model.meshes[j];

            glm::mat4 finalModelMatrix = mesh.model_matrix * model.model_matrix;
            if (!shouldSkipCulling) {
                glm::vec4 meshMin = finalModelMatrix * mesh.aabb.minPoint;
                glm::vec4 meshMax = finalModelMatrix * mesh.aabb.maxPoint;
                bool shouldDraw = camera->isInsideFrustum(meshMax, meshMin);
                if (!shouldDraw) continue;
            }

            shader.setMat4("model", finalModelMatrix);
            if (!shouldSkipTextures) {
                Material material = model.materials_loaded[mesh.materialIndex];

                if (material.textures.size() != 4) {
                    shader.setBool("noMetallicMap", true);
                    shader.setBool("noNormalMap", true);
                }
                else {
                    shader.setBool("noMetallicMap", false);
                    shader.setBool("noNormalMap", false);
                }

                for (int i = 0; i < material.textures.size(); i++) {
                    glActiveTexture(GL_TEXTURE0 + i);

                    string number;
                    string name = material.textures[i].type;

                    const string& key = name;
                    shader.setInt(key, i);

                    glBindTexture(GL_TEXTURE_2D, material.textures[i].id);
                }
                glActiveTexture(GL_TEXTURE0);

                Animation& currentAnimationData = model.animations[j];
                if (!currentAnimationData.bone_data.empty() && model.scene->mNumAnimations > 0) {
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, currentAnimationData.animationSSBO);

                    auto finalTransforms = currentAnimationData.getBoneTransforms(animationTime, model.scene, model.nodes, chosenAnimation);
                    for (unsigned int i = 0; i < finalTransforms.size(); i++) {
                        shader.setMat4("boneMatrices[" + std::to_string(i) + "]", finalTransforms[i]);
                    }
                }
            }

            glBindVertexArray(mesh.buffer.VAO);
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }
}

void BaseRenderer::loadModelData(Model& model) {
    for (auto& info : model.textures_loaded) {
        Texture& texture = info.second;
        int levels = (texture.type == "texture_normal" || texture.width < 16) ? 1 : 4;
        unsigned int textureID = glutil::createTexture(texture.width, texture.height,
            GL_UNSIGNED_BYTE, texture.nrComponents, texture.data, levels);

        texture.id = textureID;

        stbi_image_free(texture.data);
    }

    for (Material& material : model.materials_loaded) {
        for (std::string& path : material.texture_paths) {
            Texture& texture = model.textures_loaded[path];
            material.textures.push_back(texture);
        }
    }

    for (Animation& animationData: model.animations) {
        if (!animationData.bone_data.empty() && model.scene->mNumAnimations > 0) {
            glCreateBuffers(1, &animationData.animationSSBO);
            glNamedBufferStorage(animationData.animationSSBO, sizeof(VertexBoneData) * animationData.bone_data.size(),
                animationData.bone_data.data(), GL_DYNAMIC_STORAGE_BIT);
        }
    }
}

void BaseRenderer::checkFrustum(std::vector<Model>& objs) const {
    for (Model& model : objs) {
        glm::vec4 transformedMax = model.model_matrix * model.aabb.maxPoint;
        glm::vec4 transformedMin = model.model_matrix * model.aabb.minPoint;

        model.shouldDraw = camera->isInsideFrustum(transformedMax, transformedMin);
    }
}