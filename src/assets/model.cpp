#include "model.h"

#include <filesystem>

#include "stb_image.h"

#include <iostream>
#include <glm/gtx/quaternion.hpp>
#include <assimp/postprocess.h>

#include "utils/paths.h"

Model::Model() = default;

Model::Model(std::string path, FileType type) {
    size_t beginningOfPath = path.find_last_of('/');
    size_t endOfPath = path.find('.');
    std::string nameOfModel = path.substr(beginningOfPath, endOfPath - beginningOfPath);

    auto startTime = std::chrono::high_resolution_clock::now();
    std::string assetFolderPath = ASSET_PATH + nameOfModel;
    if (std::filesystem::exists(assetFolderPath)) {
        loadFromAsset(assetFolderPath);
    }
    else {
        std::string normalObjectPath = OBJECT_PATH + path;
        loadInfo(normalObjectPath, type);

        saveToAsset(assetFolderPath);
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    double elapsedTime = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    std::cout << "Elapsed Time to load model data: " << elapsedTime << " ms\n";
    model_matrix = glm::mat4(1.0f);
}

void Model::loadInfo(std::string path, FileType type) {
    int fileTypeInfo[2] = {
        aiProcess_ConvertToLeftHanded, 0
    };
    const unsigned int importerFlags =
            aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
            fileTypeInfo[type];
    Assimp::Importer importer;
    scene = importer.ReadFile(path, importerFlags);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));
    numAnimations = scene->mNumAnimations;

    processMaterials(scene);

    processNode(scene->mRootNode, scene);
    scene = importer.GetOrphanedScene();
}

void Model::saveToAsset(const std::string& assetFolderPath) {
    std::filesystem::create_directory(assetFolderPath);

    ModelAssetInfo info;
    info.numMeshes = meshes.size();
    info.numTexture = textures_loaded.size();

    assets::AssetFile file = asset_converter.convertModelAssetInfoToBinary(info);
    std::string mainFilePath = assetFolderPath + "/main.object";
    assets::saveBinaryFile(mainFilePath, file);

    std::string meshFolderPath = assetFolderPath + "/meshes";
    std::string textureFolderPath = assetFolderPath + "/textures";
    std::filesystem::create_directory(meshFolderPath);
    std::filesystem::create_directory(textureFolderPath);

    int i = 0;
    for (Mesh&mesh: meshes) {
        auto file = asset_converter.convertMeshToBinary(mesh);
        std::string assetPath = assetFolderPath + "/meshes/mesh" + std::to_string(i) + ".object";
        i++;
        bool saveSuccessful = assets::saveBinaryFile(assetPath, file);
        if (!saveSuccessful) {
            std::cout << "Error occured while saving mesh \n";
        }
    }

    i = 0;
    for (auto&[path, texture]: textures_loaded) {
        auto file = asset_converter.convertTextureToBinary(texture);

        std::string assetPath = assetFolderPath + "/textures/texture" + std::to_string(i) + ".object";
        i++;
        bool saveSuccessful = assets::saveBinaryFile(assetPath, file);
        if (!saveSuccessful) {
            std::cout << "Error occured while saving texture \n";
        }
    }
}

void Model::loadFromAsset(const std::string&assetFolderPath) {
    std::string modelInfoPath = assetFolderPath + "/main.object";
    ModelAssetInfo info = asset_converter.convertBinaryToModelAssetInfo(modelInfoPath);

    for (int i = 0; i < info.numMeshes; i++) {
        std::string meshAssetPath = assetFolderPath + "/meshes/mesh" + std::to_string(i) + ".object";
        Mesh mesh = asset_converter.convertBinaryToMesh(meshAssetPath);
        meshes.push_back(mesh);
    }

    for (int i = 0; i < info.numTexture; i++) {
        std::string textureAssetPath = assetFolderPath + "/textures/texture" + std::to_string(i) + ".object";
        Texture texture = asset_converter.convertBinaryToTexture(textureAssetPath);

        textures_loaded[textureAssetPath] = texture;
    }
}

void Model::processNode(aiNode* node, const aiScene* scene, int parentIndex) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(processMesh(mesh, scene));
    }

    NodeData data;
    data.name = std::string(node->mName.data);
    data.originalTransform = convertToGlmMatrix(node->mTransformation);
    data.parentIndex = parentIndex;
    nodes.push_back(data);
    int index = nodes.size() - 1;

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, index);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::string> textures;
    std::vector<VertexBoneData> boneData;
    std::vector<BoneInfo> boneInfo;
    std::unordered_map<std::string, unsigned int> nameToIndex;
    BoundingBox someAABB;

    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex{};
        vertex.ID = i;

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (i == 0) {
            someAABB.minPoint = glm::vec4(vector, 1.0f);
            someAABB.maxPoint = glm::vec4(vector, 1.0f);
        }
        else {
            someAABB.minPoint = glm::vec4(
                std::min(someAABB.minPoint.x, vector.x),
                std::min(someAABB.minPoint.y, vector.y),
                std::min(someAABB.minPoint.z, vector.z),
                1.0f
            );
            someAABB.maxPoint = glm::vec4(
                std::max(someAABB.maxPoint.x, vector.x),
                std::max(someAABB.maxPoint.y, vector.y),
                std::max(someAABB.maxPoint.z, vector.z),
                1.0f
            );
        }

        if (mesh->HasNormals()) {
            glm::vec3 normal;
            normal.x = mesh->mNormals[i].x;
            normal.y = mesh->mNormals[i].y;
            normal.z = mesh->mNormals[i].z;
            vertex.Normal = normal;
        }

        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;

            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }
        vertices.push_back(vertex);
    }

    if (!aabb.isInitialized) {
        aabb.isInitialized = true;
        aabb.minPoint = someAABB.minPoint;
        aabb.maxPoint = someAABB.maxPoint;
    }
    else {
        aabb.minPoint = glm::vec4(
            std::min(someAABB.minPoint.x, aabb.minPoint.x),
            std::min(someAABB.minPoint.y, aabb.minPoint.y),
            std::min(someAABB.minPoint.z, aabb.minPoint.z),
            1.0f
        );
        aabb.maxPoint = glm::vec4(
            std::max(someAABB.maxPoint.x, aabb.maxPoint.x),
            std::max(someAABB.maxPoint.y, aabb.maxPoint.y),
            std::max(someAABB.maxPoint.z, aabb.maxPoint.z),
            1.0f
        );
    }

    if (mesh->HasBones()) {
        boneData.resize(vertices.size());
        boneInfo.resize(mesh->mNumBones);
        for (unsigned int i = 0; i < mesh->mNumBones; i++) {
            aiBone* bone = mesh->mBones[i];

            std::string bone_name(bone->mName.C_Str());
            nameToIndex[bone_name] = i;

            boneInfo[i].offsetTransform = convertToGlmMatrix(bone->mOffsetMatrix);
            for (unsigned int j = 0; j < bone->mNumWeights; j++) {
                auto&weight = bone->mWeights[j];
                addBoneData(boneData[weight.mVertexId], i, weight.mWeight);
            }
        }
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    Mesh newMesh;
    newMesh.materialIndex = mesh->mMaterialIndex;

    newMesh.aabb = someAABB;
    newMesh.model_matrix = glm::mat4(1.0f);
    newMesh.indices = indices;
    newMesh.vertices = vertices;

    Animation newAnimation;
    newAnimation.bone_info = boneInfo;
    newAnimation.bone_data = boneData;
    newAnimation.boneName_To_Index = nameToIndex;
    animations.push_back(newAnimation);

    return newMesh;
}

void Model::processMaterials(const aiScene* scene) {
    std::vector<std::string> textures;
    materials_loaded.resize(scene->mNumMaterials);

    std::vector<std::pair<aiTextureType, std::string>> textureTypes = {
        {aiTextureType_DIFFUSE, "texture_diffuse"},
        {aiTextureType_SPECULAR, "texture_specular"},
        {aiTextureType_NORMALS, "texture_normal"},
        {aiTextureType_AMBIENT, "texture_height"},
        {aiTextureType_LIGHTMAP, "texture_ao"},
        {aiTextureType_METALNESS, "texture_metallic"},
        {aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness"}
    };

    for (int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial* material = scene->mMaterials[i];

        for (auto& [aiTextureType, typeName] : textureTypes) {
            auto foundTextures = loadMaterialTextures(material, aiTextureType, typeName);
            textures.insert(textures.end(), foundTextures.begin(), foundTextures.end());
        }
        materials_loaded[i].texture_paths = textures;
    }
}

std::vector<std::string> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type,
                                                     std::string typeName) {
    std::vector<std::string> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;

        auto iterator = textures_loaded.find(str.C_Str());
        if (iterator == textures_loaded.end()) {
            Texture texture;
            bool success = false;

            const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
            if (embeddedTexture) {
                success = textureFromMemory(embeddedTexture->pcData, embeddedTexture->mWidth, texture);
            }
            if (!success) {
                success = textureFromFile(str.C_Str(), directory, texture);
            }
            if (success) {
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture.path);
                textures_loaded[texture.path] = texture;
            }
        }
        else {
            textures.push_back(iterator->second.path);
        }
    }

    return textures;
}

bool textureFromMemory(void* data, unsigned int bufferSize, Texture&texture) {
    int width, height, nrComponents;
    unsigned char* image_data = stbi_load_from_memory((const stbi_uc *)data, bufferSize, &width, &height, &nrComponents,
                                                      0);

    if (image_data) {
        texture.data = image_data;
        texture.nrComponents = nrComponents;
        texture.width = width;
        texture.height = height;

        return true;
    }
    else {
        std::cout << "Embedded Texture failed to load " << std::endl;
        stbi_image_free(data);

        return false;
    }
}

bool textureFromFile(const char* path, const std::string&directory, Texture&texture, bool gamma) {
    auto filename = std::string(path);
    filename = directory + '/' + filename;

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        texture.data = data;
        texture.nrComponents = nrComponents;
        texture.width = width;
        texture.height = height;
        return true;
    }
    else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);

        return false;
    }
}

glm::mat4 convertToGlmMatrix(const aiMatrix4x4&aiMat) {
    return {
        aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
        aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
        aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
        aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
    };
}
