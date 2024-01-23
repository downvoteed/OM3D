#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct KeyFrame {
    enum class InterpolationType {
        LINEAR,
        STEP,
        // TODO
        CUBICSPLINE
    };

    float timestamp;
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    InterpolationType interpolationType;

    glm::vec3 inTangent;
    glm::vec3 outTangent;

    KeyFrame(float t, glm::vec3 pos, glm::quat rot, glm::vec3 scl)
        : timestamp(t), position(pos), rotation(rot), scale(scl) {}
};

#endif
