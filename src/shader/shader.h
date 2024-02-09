#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

inline std::map<GLint, std::string> shaderTypeToStringMap{
    {GL_VERTEX_SHADER, "VERTEX"},
    {GL_FRAGMENT_SHADER, "FRAGMENT"},
    {GL_PROGRAM, "PROGRAM"},
    {GL_GEOMETRY_SHADER, "GEOMETRTY"},
    {GL_COMPUTE_SHADER, "COMPUTE"}
};

class Shader {
public:
    unsigned int ID{};
    std::vector<std::pair<GLenum, std::string>> shaderTypesAndPaths;

    Shader();
    explicit Shader(const char* computePath);
    Shader(const char* vertexPath, const char* fragmentPath, const char* geoPath = nullptr);
    void use() const;

    void reloadShader(const char* codeBuffer, GLenum shaderType);

    void setBool(const std::string &name, bool value) const;
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const;
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const;
    // ------------------------------------------------------------------------
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec2(const std::string &name, float x, float y) const;
    // ------------------------------------------------------------------------
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    // ------------------------------------------------------------------------
    void setVec4(const std::string &name, const glm::vec4 &value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w);
    // ------------------------------------------------------------------------
    void setMat2(const std::string &name, const glm::mat2 &mat) const;
    // ------------------------------------------------------------------------
    void setMat3(const std::string &name, const glm::mat3 &mat) const;
    // ------------------------------------------------------------------------
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    void setupProgram(const vector<unsigned int>& shaderIds);

    unsigned int vertexId{}, fragmentId{}, geometryId{};
    unsigned int computeId{};
};

void openAndLoadShaderFile(const string& fullPath, string& codeBuffer);
unsigned int compileShader(GLint shaderType, const char* codeBuffer);

#endif