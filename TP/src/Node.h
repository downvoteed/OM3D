#pragma once
#include <glm/glm.hpp>

namespace OM3D
{
    struct Node
    {
        int index;
        glm::vec3 translation = glm::vec3(0.0f);
        glm::vec4 rotation = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        glm::vec3 scale = glm::vec3(1.0f);
    };
} // namespace OM3D