#ifndef VERTEX_H
#define VERTEX_H

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace OM3D {

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec4 tangent_bitangent_sign = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f); // to avoid completly black meshes if no color is present
    glm::vec<4, uint8_t> joints_0 = glm::vec<4, int>(0, 0, 0, 0);
    glm::vec4 weights_0 = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
};

}

#endif // VERTEX_H
