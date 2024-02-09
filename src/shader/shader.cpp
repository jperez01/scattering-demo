#include "shader/shader.h"

#include "utils/paths.h"

Shader::Shader() = default;

Shader::Shader(const char* computePath) {
    std::string finalComputePath = SHADER_PATH + computePath;
    std::string computeCode;
    try {
        openAndLoadShaderFile(finalComputePath, computeCode);
    } catch (ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ " << e.what() << std::endl;
    }

    unsigned int possibleComputeId = compileShader(GL_COMPUTE_SHADER, computeCode.c_str());
    setupProgram({possibleComputeId});

    computeId = possibleComputeId;

    shaderTypesAndPaths.emplace_back(GL_COMPUTE_SHADER, finalComputePath);
}

Shader::Shader(const char* vertexPath, const char* fragmentPath,
    const char* geoPath) {
    string finalVertexPath = SHADER_PATH + vertexPath;
    string finalFragmentPath = SHADER_PATH + fragmentPath;
    string finalGeometryPath;
    if (geoPath != nullptr) finalGeometryPath = SHADER_PATH + geoPath;

    string vertexCode;
    string fragmentCode;
    string geoCode;

    try {
        openAndLoadShaderFile(finalVertexPath, vertexCode);
        openAndLoadShaderFile(finalFragmentPath, fragmentCode);
        if (geoPath) {
            openAndLoadShaderFile(finalGeometryPath, geoCode);
        }
    } catch (ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ " << e.what() << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex = 0, fragment = 0, geometry = 0;

    vertex = compileShader(GL_VERTEX_SHADER, vShaderCode);
    fragment = compileShader(GL_FRAGMENT_SHADER, fShaderCode);
    if (geoPath) {
        const char* gShaderCode = geoCode.c_str();
        geometry = compileShader(GL_GEOMETRY_SHADER, gShaderCode);
    }

    vector shaderIds{ vertex, fragment };
    if (geoPath) {
        shaderIds.push_back(geometry);
    }

    setupProgram(shaderIds);

    vertexId = vertex;
    fragmentId = fragment;
    geometryId = geometry;

    shaderTypesAndPaths = { {GL_VERTEX_SHADER, finalVertexPath}, { GL_FRAGMENT_SHADER, finalFragmentPath}};
    if (geoPath) {
        shaderTypesAndPaths.emplace_back(GL_GEOMETRY_SHADER, finalGeometryPath);
    }

}

void Shader::setupProgram(const vector<unsigned>& shaderIds) {
    GLint possibleId = glCreateProgram();
    for (auto& shaderId : shaderIds) {
        glAttachShader(possibleId, shaderId);
    }
    glLinkProgram(possibleId);

    int isLinked = 0;
    glGetProgramiv(possibleId, GL_LINK_STATUS, &isLinked);
    if (!isLinked)
    {
        GLint maxLength = 1;
        glGetProgramiv(possibleId, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(possibleId, maxLength, &maxLength, &errorLog[0]);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: PROGRAM" << "\n" <<
            errorLog.data() << "\n";

        glDeleteProgram(possibleId);
        for (auto shaderId : shaderIds) {
            glDeleteShader(shaderId);
        }

        return;
    }

    for (auto shaderId : shaderIds) {
        glDetachShader(possibleId, shaderId);
    }

    ID = possibleId;
}

void openAndLoadShaderFile(const string& fullPath, string& codeBuffer) {
    ifstream shaderFile;
    shaderFile.exceptions(ifstream::failbit | ifstream::badbit);

    shaderFile.open(fullPath.c_str());
    stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    codeBuffer = shaderStream.str();
}

unsigned compileShader(GLint shaderType, const char* codeBuffer) {
    unsigned int shaderId = glCreateShader(shaderType);
    glShaderSource(shaderId, 1, &codeBuffer, nullptr);
    glCompileShader(shaderId);

    const std::string shaderTypeString = shaderTypeToStringMap[shaderType];

    GLint isCompiled = 0;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &isCompiled);

    if (!isCompiled) {
        GLint maxLength = 1;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shaderId, maxLength, &maxLength, &errorLog[0]);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << shaderTypeString << "\n" <<
            errorLog.data() << "\n";

        glDeleteShader(shaderId);
        return 0;
    }

    return shaderId;
}

void Shader::reloadShader(const char* codeBuffer, GLenum shaderType) {
    unsigned int newShaderId = compileShader(shaderType, codeBuffer);

    vector<unsigned int> shaderIds;
    if (shaderType == GL_VERTEX_SHADER) {
        shaderIds = { newShaderId, fragmentId};
        if (geometryId != 0) shaderIds.push_back(geometryId);
    } else if (shaderType == GL_FRAGMENT_SHADER) {
        shaderIds = {vertexId, newShaderId};
        if (geometryId != 0) shaderIds.push_back(geometryId);
    } else if (shaderType == GL_GEOMETRY_SHADER) {
        shaderIds = { vertexId, fragmentId, newShaderId};
    }

    setupProgram(shaderIds);

    if (shaderType == GL_VERTEX_SHADER) {
        vertexId = newShaderId;
    } else if (shaderType == GL_FRAGMENT_SHADER) {
        fragmentId = newShaderId;
    } else if (shaderType == GL_GEOMETRY_SHADER) {
        geometryId = newShaderId;
    }

    std::cout << "Reloaded Program with new Shader \n";
}


void Shader::use() const {
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}
// ------------------------------------------------------------------------
void Shader::setVec2(const std::string &name, const glm::vec2 &value) const
{ 
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec2(const std::string &name, float x, float y) const
{ 
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); 
}
// ------------------------------------------------------------------------
void Shader::setVec3(const std::string &name, const glm::vec3 &value) const
{ 
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec3(const std::string &name, float x, float y, float z) const
{ 
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); 
}
// ------------------------------------------------------------------------
void Shader::setVec4(const std::string &name, const glm::vec4 &value) const
{ 
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); 
}
void Shader::setVec4(const std::string &name, float x, float y, float z, float w)
{ 
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); 
}
// ------------------------------------------------------------------------
void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}
// ------------------------------------------------------------------------
void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}