//
// Created by jpabl on 12/21/2023.
//

#include "asset_converter.h"
#include <nlohmann/json.hpp>
#include <lz4.h>

assets::AssetFile AssetConverter::convertMeshToBinary(Mesh& mesh) {
    assets::AssetFile file;
    file.type[0] = 'M';
    file.type[1] = 'E';
    file.type[2] = 'S';
    file.type[3] = 'H';
    file.version = 1;

    nlohmann::json metadata;
    metadata["vertex_format"] = "PNTTB_F32";
    size_t vertexBufferSize = mesh.vertices.size() * sizeof(Vertex);
    size_t indexBufferSize = mesh.indices.size() * sizeof(unsigned);
    metadata["vertex_buffer_size"] = vertexBufferSize;
    metadata["indices_buffer_size"] = indexBufferSize;

    std::vector<float> boundsData;
    boundsData.resize(8);
    boundsData[0] = mesh.aabb.maxPoint.x;
    boundsData[1] = mesh.aabb.maxPoint.y;
    boundsData[2] = mesh.aabb.maxPoint.z;
    boundsData[3] = mesh.aabb.maxPoint.w;
    boundsData[4] = mesh.aabb.minPoint.x;
    boundsData[5] = mesh.aabb.minPoint.y;
    boundsData[6] = mesh.aabb.minPoint.z;
    boundsData[7] = mesh.aabb.minPoint.w;
    metadata["bounds"] = boundsData;

    std::vector<char> mergedBuffer;
    mergedBuffer.resize(vertexBufferSize + indexBufferSize);
    memcpy(mergedBuffer.data(), mesh.vertices.data(), vertexBufferSize);
    memcpy(mergedBuffer.data() + vertexBufferSize, mesh.indices.data(), indexBufferSize);

    int possibleCompressSize = LZ4_compressBound(mergedBuffer.size());
    file.binaryBlob.resize(possibleCompressSize);

    int compressedSize = LZ4_compress_default(mergedBuffer.data(), file.binaryBlob.data(), mergedBuffer.size(), possibleCompressSize);
    file.binaryBlob.resize(compressedSize);

    metadata["compression"] = "LZ4";
    file.json = metadata.dump();

    return file;
}

Mesh AssetConverter::convertBinaryToMesh(const std::string& path) {
    assets::AssetFile file;
    loadBinaryFile(path, file);

    auto metadata = nlohmann::json::parse(file.json);
    auto bounds = metadata["bounds"].get<std::vector<float>>();
    size_t vertexBufferSize = metadata["vertex_buffer_size"];
    size_t indexBufferSize = metadata["indices_buffer_size"];
    size_t totalBufferSize = vertexBufferSize + indexBufferSize;

    Mesh mesh;
    glm::vec4 maxPoint(bounds[0], bounds[1], bounds[2], bounds[3]);
    glm::vec4 minPoint(bounds[4], bounds[5], bounds[6], bounds[7]);
    mesh.aabb.maxPoint = maxPoint;
    mesh.aabb.minPoint = minPoint;

    std::vector<char> uncompressedData;
    uncompressedData.resize(totalBufferSize);
    LZ4_decompress_safe(file.binaryBlob.data(), uncompressedData.data(), file.binaryBlob.size(), totalBufferSize);

    std::vector<char> vertexBufferData;
    std::vector<char> indexBufferData;
    vertexBufferData.resize(vertexBufferSize);
    indexBufferData.resize(indexBufferSize);

    memcpy(vertexBufferData.data(), uncompressedData.data(), vertexBufferSize);
    memcpy(indexBufferData.data(), uncompressedData.data() + vertexBufferSize, indexBufferSize);

    std::vector<Vertex> vertices;
    std::vector<unsigned> indices;

    size_t vertexVectorSize = vertexBufferSize / sizeof(Vertex);
    size_t indexVectorSize = indexBufferSize / sizeof(unsigned);

    vertices.resize(vertexVectorSize);
    indices.resize(indexVectorSize);

    auto* unpackedIndices = (unsigned*)indexBufferData.data();
    for (int i = 0; i < indexVectorSize; i++) {
        indices[i] = unpackedIndices[i];
    }

    auto* unpackedVertices = (Vertex*)vertexBufferData.data();
    for (int i = 0; i < vertexVectorSize; i++) {
        vertices[i] = unpackedVertices[i];
    }

    mesh.indices = indices;
    mesh.vertices = vertices;

    return mesh;
}

assets::AssetFile AssetConverter::convertTextureToBinary(Texture& texture) {
    nlohmann::json textureMetadata;
    textureMetadata["type"] = texture.type;
    textureMetadata["format"] = "RGBA8";
    textureMetadata["width"] = texture.width;
    textureMetadata["height"] = texture.height;
    textureMetadata["nrComponents"] = texture.nrComponents;

    // TODO: Change this to account for other texture formats (Not everything will be 8 bits per channel)
    int32_t textureBufferSize = texture.height * texture.width * texture.nrComponents;
    textureMetadata["buffer_size"] = textureBufferSize;

    assets::AssetFile file;
    file.type[0] = 'T';
    file.type[1] = 'E';
    file.type[2] = 'X';
    file.type[3] = 'I';
    file.version = 1;

    int possibleCompressSize = LZ4_compressBound(textureBufferSize);
    file.binaryBlob.resize(possibleCompressSize);

    int compressedSize = LZ4_compress_default((const char*)texture.data, file.binaryBlob.data(), textureBufferSize, possibleCompressSize);
    file.binaryBlob.resize(compressedSize);

    textureMetadata["compression"] = "LZ4";
    file.json = textureMetadata.dump();

    return file;
}

Texture AssetConverter::convertBinaryToTexture(const std::string& path) {
    assets::AssetFile file;
    loadBinaryFile(path, file);

    auto metadata = nlohmann::json::parse(file.json);

    Texture texture;
    texture.height = metadata["height"];
    texture.width = metadata["width"];
    texture.nrComponents = metadata["nrComponents"];
    texture.type = metadata["type"].get<std::string>();

    int textureBufferSize = metadata["buffer_size"];
    // TODO: Fix this to not use malloc because it doesn't account for exceptions and errors
    texture.data = (unsigned char*)malloc(textureBufferSize);
    LZ4_decompress_safe(file.binaryBlob.data(), (char*)texture.data, file.binaryBlob.size(), textureBufferSize);

    return texture;
}

assets::AssetFile AssetConverter::convertModelAssetInfoToBinary(ModelAssetInfo& assetInfo) {
    nlohmann::json model_metadata;
    model_metadata["numMeshes"] = assetInfo.numMeshes;
    model_metadata["numTextures"] = assetInfo.numTexture;

    assets::AssetFile file;
    file.type[0] = 'I';
    file.type[1] = 'N';
    file.type[2] = 'F';
    file.type[3] = 'O';

    file.version = 1;
    file.json = model_metadata.dump();

    return file;
}

ModelAssetInfo AssetConverter::convertBinaryToModelAssetInfo(const std::string& path) {
    assets::AssetFile file;
    assets::loadBinaryFile(path, file);

    nlohmann::json model_metadata = nlohmann::json::parse(file.json);

    ModelAssetInfo info;
    info.numMeshes = model_metadata["numMeshes"];
    info.numTexture = model_metadata["numTextures"];

    return info;
}
