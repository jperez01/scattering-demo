//
// Created by jpabl on 12/21/2023.
//

#pragma once

#include <string>
#include <vector>

namespace assets {

struct AssetFile {
    char type[4];
    int version;
    std::string json;
    std::vector<char> binaryBlob;
};

bool saveBinaryFile(const std::string& path, const AssetFile& file);
bool loadBinaryFile(const std::string& path, AssetFile& file);
}