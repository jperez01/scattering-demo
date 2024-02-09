//
// Created by jpabl on 12/27/2023.
//

#ifndef MESH_H
#define MESH_H
#include <vector>
#include <utils/types.h>

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

    size_t materialIndex;

    glm::mat4 model_matrix;
    BoundingBox aabb;

    AllocatedBuffer buffer;
};

#endif //MESH_H
