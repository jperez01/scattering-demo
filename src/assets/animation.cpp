//
// Created by jpabl on 12/27/2023.
//

#include "animation.h"

#include <glm/gtx/quaternion.hpp>

std::vector<glm::mat4> Animation::getBoneTransforms(float time, const aiScene* scene,
                                                    std::vector<NodeData>&nodeData, int animationIndex) {
    std::vector<glm::mat4> finalTransforms(bone_info.size());
    const aiAnimation* animation = scene->mAnimations[animationIndex];

    float ticksPerSecond = animation->mTicksPerSecond != 0
                               ? animation->mTicksPerSecond
                               : 25.0f;
    float timeInTicks = time * ticksPerSecond;
    float animationTimeTicks = fmod(timeInTicks, animation->mDuration);

    glm::mat4 identity(1.0f);

    for (int i = 0; i < nodeData.size(); i++) {
        NodeData&node = nodeData[i];
        const aiNodeAnim* nodeAnim = findNodeAnim(animation, node.name);
        glm::mat4 totalTransform = node.originalTransform;

        if (nodeAnim) {
            const aiVector3D scaling = calcInterpolatedTransform(animationTimeTicks, nodeAnim->mNumScalingKeys,
                                                                 nodeAnim->mScalingKeys);
            glm::mat4 scalingMatrix = glm::scale(identity, glm::vec3(scaling.x, scaling.y, scaling.z));

            const aiQuaternion rotationQ = calcInterpolatedRotation(animationTimeTicks, nodeAnim);
            glm::quat rotateQ(rotationQ.w, rotationQ.x, rotationQ.y, rotationQ.z);
            glm::mat4 rotationMatrix = glm::toMat4(rotateQ);

            const aiVector3D translation = calcInterpolatedTransform(animationTimeTicks, nodeAnim->mNumPositionKeys,
                                                                     nodeAnim->mPositionKeys);
            glm::mat4 translationMatrix = glm::translate(
                identity, glm::vec3(translation.x, translation.y, translation.z));

            totalTransform = translationMatrix * rotationMatrix * scalingMatrix;
        }

        glm::mat4 parentTrasform = node.parentIndex == -1 ? identity : nodeData[node.parentIndex].transformation;
        node.transformation = parentTrasform * totalTransform;

        if (boneName_To_Index.find(node.name) != boneName_To_Index.end()) {
            unsigned int boneIndex = boneName_To_Index[node.name];
            finalTransforms[boneIndex] = node.transformation * bone_info[boneIndex].offsetTransform;
        }
    }

    return finalTransforms;
}

aiVector3D calcInterpolatedTransform(float animationTicks, unsigned numKeys, aiVectorKey* keys) {
    if (numKeys == 1) {
        return keys[0].mValue;
    }

    unsigned int transformIndex = 0;
    for (unsigned int i = 0; i < numKeys - 1; i++) {
        if (animationTicks < keys[i + 1].mTime) {
            transformIndex = i;
            break;
        }
    }

    unsigned int nextTransformIndex = transformIndex + 1;
    double time1 = keys[transformIndex].mTime, time2 = keys[nextTransformIndex].mTime;

    double deltaTime = time2 - time1;
    float factor = (animationTicks - time1) / deltaTime;

    const aiVector3D &start = keys[transformIndex].mValue,
            end = keys[nextTransformIndex].mValue;
    const aiVector3D delta = end - start;
    return start + factor * delta;
}

aiQuaternion calcInterpolatedRotation(float animationTicks, const aiNodeAnim* nodeAnim) {
    if (nodeAnim->mNumRotationKeys == 1) {
        return nodeAnim->mRotationKeys[0].mValue;
    }

    unsigned int rotationIndex = 0;
    for (unsigned int i = 0; i < nodeAnim->mNumRotationKeys - 1; i++) {
        float t = nodeAnim->mRotationKeys[i + 1].mTime;
        if (animationTicks < t) {
            rotationIndex = i;
            break;
        }
    }
    unsigned int nextRotationIndex = rotationIndex + 1;
    assert(nextRotationIndex < nodeAnim->mNumRotationKeys);

    float t1 = nodeAnim->mRotationKeys[rotationIndex].mTime,
            t2 = nodeAnim->mRotationKeys[nextRotationIndex].mTime;

    float deltaTime = t2 - t1;
    float factor = (animationTicks - t1) / deltaTime;
    const aiQuaternion &start = nodeAnim->mRotationKeys[rotationIndex].mValue,
            end = nodeAnim->mRotationKeys[nextRotationIndex].mValue;

    aiQuaternion output{};
    aiQuaternion::Interpolate(output, start, end, factor);
    return output.Normalize();
}

const aiNodeAnim* findNodeAnim(const aiAnimation* animation, const std::string&nodeName) {
    for (unsigned int i = 0; i < animation->mNumChannels; i++) {
        const aiNodeAnim* nodeAnim = animation->mChannels[i];

        if (std::string(nodeAnim->mNodeName.data) == nodeName) {
            return nodeAnim;
        }
    }
    return nullptr;
}

void addBoneData(VertexBoneData&data, unsigned int boneID, float weight) {
    for (unsigned int i = 0; i < MAX_BONES_PER_VERTEX; i++) {
        if (data.boneIDs[i] == boneID) return;
    }
    if (weight == 0.0f) return;

    for (unsigned int i = 0; i < MAX_BONES_PER_VERTEX; i++) {
        if (data.weights[i] == 0.0f) {
            data.boneIDs[i] = boneID;
            data.weights[i] = weight;
            return;
        }
    }
}
