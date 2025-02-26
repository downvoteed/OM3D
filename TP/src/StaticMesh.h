#ifndef STATICMESH_H
#define STATICMESH_H

#include <graphics.h>
#include <TypedBuffer.h>
#include <Vertex.h>
#include <math.h>

#include <vector>

namespace OM3D {

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<u32> indices;
};


class Sphere
{
    private:
        Vertex center_;
        double radius_;

    public:

        Sphere(const std::vector<Vertex> &_vertex_buffer)
        {
            auto minVertex = _vertex_buffer[0];
            Vertex maxVertex = _vertex_buffer[0];

            for (const auto &v : _vertex_buffer)
            {
                minVertex.position.x = std::min(minVertex.position.x, v.position.x);
                minVertex.position.y = std::min(minVertex.position.y, v.position.y);
                minVertex.position.z = std::min(minVertex.position.z, v.position.z);

                maxVertex.position.x = std::max(maxVertex.position.x, v.position.x);
                maxVertex.position.y = std::max(maxVertex.position.y, v.position.y);
                maxVertex.position.z = std::max(maxVertex.position.z, v.position.z);
            }

            Vertex center;
            center.position.x = (minVertex.position.x + maxVertex.position.x) / 2;
            center.position.y = (minVertex.position.y + maxVertex.position.y) / 2;
            center.position.z = (minVertex.position.z + maxVertex.position.z) / 2;

            double radius = 0.0f;
            for (const auto &v : _vertex_buffer)
            {
                double dist = sqrt(pow(v.position.x - center.position.x, 2) 
                + pow(v.position.y - center.position.y, 2) + pow(v.position.z - center.position.z, 2));
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

class StaticMesh : NonCopyable {

    public:
        StaticMesh() = default;
        StaticMesh(StaticMesh&&) = default;
        StaticMesh& operator=(StaticMesh&&) = default;

        StaticMesh(const MeshData& data);
        Sphere get_bounding_sphere() const;

        void draw() const;

        private:
            TypedBuffer<Vertex> _vertex_buffer;
            TypedBuffer<u32> _index_buffer;
            Sphere _bounding_sphere;
        };
}

#endif // STATICMESH_H
