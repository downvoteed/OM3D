#ifndef SCENE_H
#define SCENE_H

#include <SceneObject.h>
#include <PointLight.h>
#include <Camera.h>
#include <unordered_map>

#include <vector>
#include <memory>

namespace OM3D {

class Scene : NonMovable {

    public:
        Scene();

        static Result<std::unique_ptr<Scene>> from_gltf(const std::string& file_name);

        void render() const;
        void renderLights() const;
        void renderAnimators() const;
        void renderMotionBlur() const;
        
        void add_object(SceneObject obj);
        void add_light(PointLight obj);

        Span<const SceneObject> objects() const;
        Span<const PointLight> point_lights() const;
        std::vector<AnimationChannel>& animators();

        Camera& camera();
        const Camera& camera() const;

        void set_sun(glm::vec3 direction, glm::vec3 color = glm::vec3(1.0f));
        void set_animators(std::vector<AnimationChannel>& animators);

    private:
        std::vector<SceneObject> _objects;
        std::vector<PointLight> _point_lights;
        std::vector<AnimationChannel> _animators;

        glm::vec3 _sun_direction = glm::vec3(0.2f, 1.0f, 0.1f);
        glm::vec3 _sun_color = glm::vec3(1.0f);

        // map obj name to obj index
        std::unordered_map<std::string, size_t> _obj_name_to_index;


        Camera _camera;
};

}

#endif // SCENE_H
