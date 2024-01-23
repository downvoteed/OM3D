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
            for (int i = 0; i < size; i++)
            {
                jointmatrix[i] = _mesh->get_skeleton()->joints()[i];
            }

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
} // namespace OM3D
