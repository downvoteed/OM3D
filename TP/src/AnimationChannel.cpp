#include "AnimationChannel.h"

namespace OM3D
{
    AnimationChannel::AnimationChannel(AnimationSampler sampler, Node node,
                                       PathType pathType)
        : _sampler(sampler)
        , _node(node)
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

    Node AnimationChannel::node() const
    {
        return this->_node;
    }

    void AnimationChannel::set_node(Node node)
    {
        this->_node = node;
    }

    PathType AnimationChannel::path_type() const
    {
        return this->_pathType;
    }

    void AnimationChannel::set_path_type(PathType pathType)
    {
        this->_pathType = pathType;
    }

    void AnimationChannel::update()
    {
        glm::vec4 output = _sampler.update();
        switch (_pathType)
        {
        case TRANSLATION:
            _node.translation = glm::vec3(output.x, output.y, output.z);
            break;
        case ROTATION:
            _node.rotation = glm::vec4(output.x, output.y, output.z, output.w);
            break;
        case SCALE:
            _node.scale = glm::vec3(output.x, output.y, output.z);
            break;
        default:
            break;
        }
    }

} // namespace OM3D