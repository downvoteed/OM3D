#include "SceneObject.h"

#include <glm/gtc/matrix_transform.hpp>

#include <iostream>


namespace OM3D {

SceneObject::SceneObject(std::shared_ptr<StaticMesh> mesh, std::shared_ptr<Material> material) :
    _mesh(std::move(mesh)),
    _material(std::move(material)) {
}

Sphere SceneObject::get_bounding_sphere() const 
{
    return this->_mesh->get_bounding_sphere();
}

void SceneObject::render() const {
    if(!_material || !_mesh) {
        return;
    }

    _material->set_uniform(HASH("model"), transform());

    int size = _mesh->get_skeleton().joints().size() < 100 ? _mesh->get_skeleton().joints().size() : 100;
    glm::mat4 jointmatrix[100];
    for (int i = 0; i < size; i++)
    {
        jointmatrix[i] = _mesh->get_skeleton().joints()[i];
    }

    _material->set_uniform(HASH("u_joint_matrix"), jointmatrix);

    _material->bind();
    _mesh->draw();
}

void SceneObject::set_transform(const glm::mat4& tr) {
    _transform = tr;
}

const glm::mat4& SceneObject::transform() const {
    return _transform;
}

}
