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
    
    // Get global transform matrix
    glm::mat4 global_transform = transform();
    glm::mat4 inverse_global_transform = glm::inverse(global_transform);

    // Get global Joint Transform : it transforms the mesh from the local space of the joint, based ont the current global transform of the joint
    glm::mat4 global_joint_transform = global_transform * _mesh->get_skeleton().inverse_bind_matrix();

    // Get joints list
    std::vector<int> joints = _mesh->get_skeleton().joints();
    
    // A jointMatrix may be compute for each joint    
    glm::mat4 joint_matrix = inverse_global_transform * global_joint_transform * _mesh->get_skeleton().inverse_bind_matrix();

    // Set joint matrices uniform
    _material->set_uniform(HASH("u_joint_matrix"), joint_matrix);

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
