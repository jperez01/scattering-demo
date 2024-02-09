//
// Created by jpabl on 12/2/2023.
//

#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

#include "shader.h"
#include "efsw/efsw.hpp"

struct UpdateShaderInfo {
    std::string shaderCodeBuffer;
    GLenum shaderType;
    Shader& shaderProgram;
};

class UpdateListener : public efsw::FileWatchListener {
public:
    void handleFileAction(efsw::WatchID watchid, const std::string& dir,
        const std::string& filename, efsw::Action action, std::string oldFilename) override;
    void subscribe(Shader& shaderProgram);
    void handleUpdates();

private:
    std::map<std::string, std::pair<GLenum, Shader&>> shaderPathToProgramMap;
    std::vector<UpdateShaderInfo> shadersToUpdate;
};



#endif //FILE_WATCHER_H
