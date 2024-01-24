#include "AnimationSampler.h"
#include <GLFW/glfw3.h>
#include <iostream>

namespace OM3D
{
    AnimationSampler::AnimationSampler(
        std::vector<float> inputs, std::vector<glm::vec4> outputs,
        InterpolationType::Type interpolationType)
        : _inputs(inputs)
    , _outputs(outputs)
    , _interpolationType(interpolationType)
    {}

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

    glm::vec4 AnimationSampler::update()
    {
        // get the current time of the program
        double currentTime = glfwGetTime();
        currentTime -= _time;
        // if the index is at the end of the vector, reset the time and index
        if (_index >= _inputs.size() - 1)
        {
            _time = glfwGetTime();
            currentTime = 0.0001f;
            _index = 0;
        }
        float previousTime = _inputs[_index];
        float nextTime = _inputs[_index + 1];

        glm::vec4 previousOutput = _outputs[_index];
        glm::vec4 nextOutput = _outputs[_index + 1];

        _index++;

        if (_interpolationType == InterpolationType::Type::LINEAR)
        {
            float interpolationValue = (currentTime - previousTime) / (nextTime - previousTime);
            glm::vec4 output = previousOutput + interpolationValue * (nextOutput - previousOutput);
            //std::cout << "output: " << output.x << " " << output.y << " " << output.z << " " << output.w << std::endl;
            return output;
        }
        else if (_interpolationType == InterpolationType::Type::STEP)
        {
            return previousOutput;
        }
        else if (_interpolationType == InterpolationType::Type::CUBICSPLINE)
        {
            // TODO
            return glm::vec4(0.0f);
        }
        else
        {
            return glm::vec4(0.0f);
        }
    }

} // namespace OM3D