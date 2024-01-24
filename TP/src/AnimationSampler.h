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
        //AnimationSampler() = delete;
        //AnimationSampler(const AnimationSampler&) = delete;
        AnimationSampler(std::vector<float> inputs,
                         std::vector<glm::vec4> outputs,
                         InterpolationType::Type interpolationType);

        std::vector<float> inputs() const;
        void set_inputs(std::vector<float> inputs);

        std::vector<glm::vec4> outputs() const;
        void set_outputs(std::vector<glm::vec4> outputs);

        InterpolationType::Type interpolation_type() const;
        void set_interpolation_type(InterpolationType::Type interpolationType);

        glm::vec4 update();

    private:
        int _index = 0;
        double _time = 0.0f;
        std::vector<float> _inputs;
        std::vector<glm::vec4> _outputs;
        InterpolationType::Type _interpolationType;
    };
} // namespace OM3D