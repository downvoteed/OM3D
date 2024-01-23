#pragma once
#include <glm/glm.hpp>

namespace OM3D
{
    struct Node
    {
        int index;
        glm::vec3 translation;
        glm::vec4 rotation;
        glm::vec3 scale;
    };
} // namespace OM3D