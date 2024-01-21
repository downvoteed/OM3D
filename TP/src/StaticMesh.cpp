#include "StaticMesh.h"

#include <glad/glad.h>

namespace OM3D {
    extern bool audit_bindings_before_draw;

    void Skeleton::set_inverse_bind_matrix(const glm::mat4& ibm) {
        _inverseBindMatrix = ibm;
    }

    const glm::mat4& Skeleton::inverse_bind_matrix() const {
        return _inverseBindMatrix;
    }

    void Skeleton::add_joint(glm::mat4 joint) {
        _joints.push_back(joint);
    }

    void Skeleton::set_joints(const std::vector<glm::mat4>& joints) {
        _joints = joints;
    }

    const std::vector<glm::mat4>& Skeleton::joints() const {
        return _joints;
    }


    StaticMesh::StaticMesh(const MeshData& data) : _vertex_buffer(data.vertices),
                                                   _index_buffer(data.indices),
                                                   _bounding_sphere(data.vertices) {
    }

    Sphere StaticMesh::get_bounding_sphere() const {
        return _bounding_sphere;
    }

    void StaticMesh::draw() const {
        _vertex_buffer.bind(BufferUsage::Attribute);
        _index_buffer.bind(BufferUsage::Index);

        // Vertex position
        glVertexAttribPointer(0, 3, GL_FLOAT, false, sizeof(Vertex), nullptr);
        // Vertex normal
        glVertexAttribPointer(1, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void *>(3 * sizeof(float)));
        // Vertex uv
        glVertexAttribPointer(2, 2, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void *>(6 * sizeof(float)));
        // Tangent / bitangent sign
        glVertexAttribPointer(3, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void *>(8 * sizeof(float)));
        // Vertex color
        glVertexAttribPointer(4, 3, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void *>(12 * sizeof(float)));
        // // Vertex joints (UINT)
        glVertexAttribIPointer(5, 4, GL_UNSIGNED_BYTE, sizeof(Vertex), reinterpret_cast<void *>(15 * sizeof(float)));
        // // Vertex weights
        glVertexAttribPointer(6, 4, GL_FLOAT, false, sizeof(Vertex), reinterpret_cast<void *>(15 * sizeof(float) + 4 * sizeof(uint8_t)));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glEnableVertexAttribArray(3);
        glEnableVertexAttribArray(4);
        glEnableVertexAttribArray(5);
        glEnableVertexAttribArray(6);

        if (audit_bindings_before_draw) {
            audit_bindings();
        }


        glDrawElements(GL_TRIANGLES, int(_index_buffer.element_count()), GL_UNSIGNED_INT, nullptr);
    }

    void StaticMesh::set_skeleton(const Skeleton& skel) {
        _skeleton = skel;
    }

    const Skeleton& StaticMesh::get_skeleton() const {
        return _skeleton;
    }
}
