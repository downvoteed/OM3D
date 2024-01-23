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

} // namespace OM3D