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
        //AnimationChannel() = delete;
        //AnimationChannel(const AnimationChannel&) = delete;
        //AnimationChannel& operator=(const AnimationChannel&) = delete;
        AnimationChannel(AnimationSampler sampler, int nodeIndex, PathType pathType);

        void update(Node& node);

        AnimationSampler sampler() const;
        void set_sampler(AnimationSampler sampler);

        int nodeIndex() const;
        void set_nodeIndex(int nodeIndex);

        PathType path_type() const;
        void set_path_type(PathType pathType);
    private:
        AnimationSampler _sampler;
        int _nodeIndex;
        PathType _pathType;

    };
} // namespace OM3D