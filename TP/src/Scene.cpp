#include "Scene.h"

#include <TypedBuffer.h>

#include <shader_structs.h>
#include <iostream>
#include <unordered_map>
#include <set>

namespace OM3D {

Scene::Scene() {
}

void Scene::add_object(SceneObject obj) {
    _objects.emplace_back(std::move(obj));
}

void Scene::add_light(PointLight obj) {
    _point_lights.emplace_back(std::move(obj));
}

Span<const SceneObject> Scene::objects() const {
    return _objects;
}

Span<const PointLight> Scene::point_lights() const {
    return _point_lights;
}

Camera& Scene::camera() {
    return _camera;
}

const Camera& Scene::camera() const {
    return _camera;
}

void Scene::set_sun(glm::vec3 direction, glm::vec3 color) {
    _sun_direction = direction;
    _sun_color = color;
}

void Scene::render() const {
    // Fill and bind frame data buffer
    TypedBuffer<shader::FrameData> buffer(nullptr, 1);
    {
        auto mapping = buffer.map(AccessType::WriteOnly);
        mapping[0].camera.view_proj = _camera.view_proj_matrix();
        mapping[0].point_light_count = u32(_point_lights.size());
        mapping[0].sun_color = _sun_color;
        mapping[0].sun_dir = glm::normalize(_sun_direction);
    }
    buffer.bind(BufferUsage::Uniform, 0);

    // Fill and bind lights buffer
    TypedBuffer<shader::PointLight> light_buffer(nullptr, std::max(_point_lights.size(), size_t(1)));
    {
        auto mapping = light_buffer.map(AccessType::WriteOnly);
        for(size_t i = 0; i != _point_lights.size(); ++i) {
            const auto& light = _point_lights[i];
            mapping[i] = {
                light.position(),
                light.radius(),
                light.color(),
                0.0f
            };
        }
    }
    light_buffer.bind(BufferUsage::Storage, 1);

    for (const auto& obj: this->_obj_name_to_index)
    {
        std::cout << obj.first << std::endl;
    }

    std::unordered_map<std::string, std::set<const SceneObject*>> cluster_objects;

    int i = 0;
    for (const auto &obj_pair : this->_obj_name_to_index)
    {
        std::string basename;

        for (char ch : obj_pair.first)
        {
            if (std::isalpha(ch))
            {
                basename += ch;
            }
            else
            {
                break;
            }
        }

        // Ajouter l'objet au set correspondant à son nom de base
        cluster_objects[basename].insert(&_objects[i]);
        i++;
    }

    for (const auto &pair : cluster_objects)
    {
        std::cout << "Nom de base: " << pair.first << " - Objets: ";
        int j = 0;
        for (const SceneObject *obj : pair.second)
        {
            std::cout << "Objet " << j << ": " << std::endl;
            j++;
        }
        std::cout << std::endl;
    }

    // Render every object
    for (const SceneObject &obj : _objects)
    {
        Frustum frustum = this->_camera.build_frustum();

        bool is_in_frustum = this->_camera.isInside(obj.get_bounding_sphere(), frustum);

        // cluster the objects having the same basename in a set

        // std::cout << is_in_frustum << std::endl;

        if (is_in_frustum)
            obj.render();
    }
}

}
