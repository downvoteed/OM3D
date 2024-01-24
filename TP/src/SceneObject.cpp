#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace OM3D
{
    SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh,
                             std::shared_ptr<Material> material)
        : _mesh(std::move(mesh))
        , _material(std::move(material))
    {}

    Sphere SceneObject::get_bounding_sphere() const
    {
        return this->_mesh->get_bounding_sphere();
    }

    Skeleton* SceneObject::get_skeleton() const
    {
        return this->_mesh->get_skeleton().get();
    }

    void SceneObject::render() const
    {
        if (!_material || !_mesh)
        {
            return;
        }

        _material->set_uniform(HASH("model"), transform());

        if (const auto skeleton = _mesh->get_skeleton(); !skeleton)
        {
            _material->set_uniform(HASH("u_has_skeleton"), false);
        }
        else
        {
            auto size = skeleton->joints().size();
            glm::mat4 jointmatrix[size];
            int i = 0;
            for (auto [nodeIndex, matrix] : skeleton->joints())
            {
                jointmatrix[i++] = matrix;
            }
            //std::cout << "matrix:" << jointmatrix[0][0][0] << std::endl;

            _material->set_uniform(HASH("u_joint_matrix"), &jointmatrix[0],
                                   size);
            _material->set_uniform(HASH("u_has_skeleton"), true);
        }

        _material->bind();
        _mesh->draw();
    }

    void SceneObject::set_transform(const glm::mat4& tr)
    {
        _transform = tr;
    }

    const glm::mat4& SceneObject::transform() const
    {
        return _transform;
    }

    void SceneObject::updateJointMatrix()
    {
        if (const auto skeleton = _mesh->get_skeleton(); !skeleton)
        {
            return;
        }
        else
        {
            for (auto &[index, node] : skeleton->nodes())
            {
                const glm::tquat<float> q(node.rotation.w, node.rotation.x, node.rotation.y, node.rotation.z);
                glm::mat4 nodeTransform = glm::translate(glm::mat4(1.0f), node.translation) * glm::mat4_cast(q) * glm::scale(glm::mat4(1.0f), node.scale);
                glm::mat4 inverseNodeTransform = glm::inverse(nodeTransform);
                glm::mat4 jointTransform = skeleton->inverse_bind_matrix() * nodeTransform;
                skeleton->joints().at(index) = inverseNodeTransform * jointTransform;
            }
            /* //check if the node is in the skeleton
            if (skeleton->joints().find(nodeIndex) != skeleton->joints().end())
            {
                Node& node = skeleton->nodes().at(nodeIndex);
                // compute node transformation matrix             
                const glm::tquat<float> q(node.rotation.w, node.rotation.x, node.rotation.y, node.rotation.z);
                glm::mat4 nodeTransform = glm::translate(glm::mat4(1.0f), node.translation) * glm::mat4_cast(q) * glm::scale(glm::mat4(1.0f), node.scale);
                glm::mat4 inverseNodeTransform = glm::inverse(nodeTransform);
                glm::mat4 jointTransform = skeleton->inverse_bind_matrix() * nodeTransform;
                skeleton->joints().at(nodeIndex) = inverseNodeTransform * jointTransform;
            } */
        }
    }

} // namespace OM3D
