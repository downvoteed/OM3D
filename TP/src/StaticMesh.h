#ifndef STATICMESH_H
#define STATICMESH_H

#include <graphics.h>
#include <TypedBuffer.h>
#include <Vertex.h>
#include <math.h>
#include <memory>
#include <map>

#include <glm/gtc/matrix_transform.hpp>
#include "AnimationChannel.h"

#include <vector>

namespace OM3D
{

    class Skeleton
    {
    public:
        Skeleton(const glm::mat4& inverseBindMatrix,
                 const std::map<int, glm::mat4>& joints)
            : _inverseBindMatrix(inverseBindMatrix)
            , _joints(joints)
        {}
        void set_inverse_bind_matrix(const glm::mat4& ibm);
        const glm::mat4& inverse_bind_matrix() const;

        void add_joint(int index, glm::mat4& joint);
        void set_joints(const std::map<int, glm::mat4>& joints);
        void set_animators(const std::vector<AnimationSampler>& animators);
        const std::vector<AnimationChannel>& get_animators() const;
        const std::map<int, glm::mat4>& joints() const;

    private:
        glm::mat4 _inverseBindMatrix = glm::mat4(1.0f);
        std::map<int, glm::mat4> _joints;
        std::vector<AnimationChannel> _animators;
    };

    struct MeshData
    {
        std::vector<Vertex> vertices;
        std::vector<u32> indices;
    };

    class Sphere
    {
    private:
        Vertex center_;
        double radius_;

    public:
        Sphere(const std::vector<Vertex>& _vertex_buffer)
        {
            auto [position, normal, uv, tangent_bitangent_sign, color, joints_0,
                  weights_0] = _vertex_buffer[0];
            Vertex maxVertex = _vertex_buffer[0];

            for (const auto& v : _vertex_buffer)
            {
                position.x = std::min(position.x, v.position.x);
                position.y = std::min(position.y, v.position.y);
                position.z = std::min(position.z, v.position.z);

                maxVertex.position.x =
                    std::max(maxVertex.position.x, v.position.x);
                maxVertex.position.y =
                    std::max(maxVertex.position.y, v.position.y);
                maxVertex.position.z =
                    std::max(maxVertex.position.z, v.position.z);
            }

            Vertex center;
            center.position.x = (position.x + maxVertex.position.x) / 2;
            center.position.y = (position.y + maxVertex.position.y) / 2;
            center.position.z = (position.z + maxVertex.position.z) / 2;

            double radius = 0.0f;
            for (const auto& v : _vertex_buffer)
            {
                double dist = sqrt(pow(v.position.x - center.position.x, 2)
                                   + pow(v.position.y - center.position.y, 2)
                                   + pow(v.position.z - center.position.z, 2));
                radius = std::max(radius, dist);
            }

            this->center_ = center;
            this->radius_ = radius;
        }

        Vertex center() const
        {
            return this->center_;
        }
        double radius() const
        {
            return this->radius_;
        }
    };

    class StaticMesh : NonCopyable
    {
    public:
        StaticMesh() = default;
        StaticMesh(StaticMesh&&) = default;
        StaticMesh& operator=(StaticMesh&&) = default;

        StaticMesh(const MeshData& data);
        Sphere get_bounding_sphere() const;

        void draw() const;

        [[nodiscard]] const std::shared_ptr<Skeleton>& get_skeleton() const;
        void set_skeleton(const std::shared_ptr<Skeleton>& skel);

    private:
        TypedBuffer<Vertex> _vertex_buffer;
        TypedBuffer<u32> _index_buffer;
        Sphere _bounding_sphere;

        std::shared_ptr<Skeleton> _skeleton;
    };
} // namespace OM3D

#endif // STATICMESH_H
