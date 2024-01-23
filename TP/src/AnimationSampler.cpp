#include "AnimationSampler.h"

namespace OM3D
{
    AnimationSampler::AnimationSampler(
        std::vector<float> inputs, std::vector<glm::vec4> outputs,
        InterpolationType::Type interpolationType)
        : this->_inputs(inputs)
    , this->_outputs(outputs)
    , this->_interpolationType(interpolationType)
    {}

    int AnimationSampler::index() const
    {
        return this->_index;
    }

    void AnimationSampler::setthis->_index(int index)
    {
        this->_index = index;
    }

    std::vector<float> AnimationSampler::inputs() const
    {
        return this->_inputs;
    }

    void AnimationSampler::set_inputs(std::vector<float> inputs)
    {
        this->_inputs = inputs;
    }

    std::vector<glm::vec4> AnimationSampler::outputs() const
    {
        return this->_outputs;
    }

    void AnimationSampler::set_outputs(std::vector<glm::vec4> outputs)
    {
        this->_outputs = outputs;
    }

    InterpolationType::Type AnimationSampler::interpolation_type() const
    {
        return this->_interpolationType;
    }

    void AnimationSampler::set_interpolation_type(
        InterpolationType::Type interpolationType)
    {
        this->_interpolationType = interpolationType;
    }

} // namespace OM3D