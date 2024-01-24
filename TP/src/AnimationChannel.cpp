#include "AnimationChannel.h"

namespace OM3D
{
    AnimationChannel::AnimationChannel(AnimationSampler sampler, int nodeIndex,
                                       PathType pathType)
        : _sampler(sampler)
        , _nodeIndex(nodeIndex)
        , _pathType(pathType)
    {}

    AnimationSampler AnimationChannel::sampler() const
    {
        return this->_sampler;
    }

    void AnimationChannel::set_sampler(AnimationSampler sampler)
    {
        this->_sampler = sampler;
    }

    int AnimationChannel::nodeIndex() const
    {
        return this->_nodeIndex;
    }

    void AnimationChannel::set_nodeIndex(int nodeIndex)
    {
        this->_nodeIndex = nodeIndex;
    }

    PathType AnimationChannel::path_type() const
    {
        return this->_pathType;
    }

    void AnimationChannel::set_path_type(PathType pathType)
    {
        this->_pathType = pathType;
    }

    void AnimationChannel::update(Node& node)
    {
        glm::vec4 output = _sampler.update();
        switch (_pathType)
        {
        case TRANSLATION:
            node.translation = glm::vec3(output.x, output.y, output.z);
            break;
        case ROTATION:
            node.rotation = glm::vec4(output.x, output.y, output.z, output.w);
            break;
        case SCALE:
            node.scale = glm::vec3(output.x, output.y, output.z);
            break;
        default:
            break;
        }
    }

} // namespace OM3D