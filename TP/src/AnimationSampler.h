#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace OM3D
{
    struct InterpolationType
    {
        enum class Type
        {
            LINEAR,
            STEP,
            // TODO
            CUBICSPLINE
        };
    };

    class AnimationSampler
    {
    public:
        AnimationSampler() = delete;
        AnimationSampler(const AnimationSampler&) = delete;
        AnimationSampler(std::vector<float> inputs,
                         std::vector<glm::vec4> outputs,
                         InterpolationType::Type interpolationType);

    private:
        int _index;
        std::vector<float> _inputs;
        std::vector<glm::vec4> _outputs;
        InterpolationType::Type _interpolationType;
    };
} // namespace OM3D