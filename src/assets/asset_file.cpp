//
// Created by jpabl on 12/21/2023.
//

#include "asset_file.h"
#include <fstream>

bool assets::saveBinaryFile(const std::string& path, const AssetFile& file) {
    std::ofstream outputFile;
    outputFile.open(path, std::ios::binary | std::ios::out);

    outputFile.write(file.type, 4);

    const uint32_t version = file.version;
    outputFile.write((const char*) &version, sizeof(uint32_t));

    const uint32_t length = file.json.size();
    outputFile.write((const char*) &length, sizeof(uint32_t));

    const uint32_t blobSize = file.binaryBlob.size();
    outputFile.write((const char*) &blobSize, sizeof(uint32_t));

    outputFile.write(file.json.c_str(), length);

    outputFile.write(file.binaryBlob.data(), blobSize);

    outputFile.close();

    return true;
}

bool assets::loadBinaryFile(const std::string& path, AssetFile& file) {
    std::ifstream inputFile;
    inputFile.open(path, std::ios::binary);

    if (!inputFile.is_open()) {
        return false;
    }

    inputFile.seekg(0);

    inputFile.read(file.type, 4);
    inputFile.read((char*) &file.version, sizeof(uint32_t));

    uint32_t jsonLength = 0;
    inputFile.read((char*) &jsonLength, sizeof(uint32_t));
    uint32_t blobSize = 0;
    inputFile.read((char*) &blobSize, sizeof(uint32_t));

    file.json.resize(jsonLength);
    inputFile.read(file.json.data(), jsonLength);

    file.binaryBlob.resize(blobSize);
    inputFile.read(file.binaryBlob.data(), blobSize);

    return true;
}
