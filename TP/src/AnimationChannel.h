#pragma once
#include "AnimationSampler.h"
#include "Node.h"

namespace OM3D
{
    enum PathType
    {
        TRANSLATION,
        ROTATION,
        SCALE
    };

    class AnimationChannel
    {
    public:
        AnimationChannel() = delete;
        AnimationChannel(const AnimationChannel&) = delete;
        AnimationChannel& operator=(const AnimationChannel&) = delete;
        AnimationChannel(AnimationSampler sampler, Node node, PathType pathType);

        AnimationSampler sampler() const;
        void set_sampler(AnimationSampler sampler);

        Node node() const;
        void set_node(Node node);

        PathType path_type() const;
        void set_path_type(PathType pathType);
    private:
        AnimationSampler _sampler;
        Node _node;
        PathType _pathType;

    };
} // namespace OM3D