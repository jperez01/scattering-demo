//
// Created by jpabl on 12/27/2023.
//

#ifndef ANIMATION_H
#define ANIMATION_H
#include <assimp/scene.h>
#include <utils/types.h>

struct NodeData {
    glm::mat4 transformation;
    glm::mat4 originalTransform;
    std::string name;
    int parentIndex;
};

struct Animation {
    std::vector<VertexBoneData> bone_data;
    std::vector<BoneInfo> bone_info;
    std::unordered_map<std::string, unsigned int> boneName_To_Index;

    unsigned int animationSSBO;

    std::vector<glm::mat4> getBoneTransforms(float time, const aiScene* scene, std::vector<NodeData>&nodeData,
                                             int animationIndex = 0);
};

aiVector3D calcInterpolatedTransform(float animationTicks, unsigned int numKeys, aiVectorKey* keys);

aiQuaternion calcInterpolatedRotation(float animationTicks, const aiNodeAnim* nodeAnim);

const aiNodeAnim* findNodeAnim(const aiAnimation* animation, const std::string&nodeName);

void addBoneData(VertexBoneData&data, unsigned int boneID, float weight);
#endif //ANIMATION_H
