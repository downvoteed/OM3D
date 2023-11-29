#ifndef CAMERA_H
#define CAMERA_H

#include <glm/gtc/matrix_transform.hpp>

#include <utils.h>
#include <StaticMesh.h>
#include <iostream>

namespace OM3D {

struct Frustum {
    glm::vec3 _near_normal;
    glm::vec3 _top_normal;
    glm::vec3 _bottom_normal;
    glm::vec3 _right_normal;
    glm::vec3 _left_normal;

    bool isInside(const Sphere& sphere) const {

        std::cout << "Sphere center: " << sphere.center().position.x << " " << sphere.center().position.y << " " << sphere.center().position.z << std::endl;
        std::cout << "Sphere radius: " << sphere.radius() << std::endl;
        std::cout << "Near normal: " << _near_normal.x << " " << _near_normal.y << " " << _near_normal.z << std::endl;
        std::cout << "Top normal: " << _top_normal.x << " " << _top_normal.y << " " << _top_normal.z << std::endl;
        std::cout << "Bottom normal: " << _bottom_normal.x << " " << _bottom_normal.y << " " << _bottom_normal.z << std::endl;
        std::cout << "Right normal: " << _right_normal.x << " " << _right_normal.y << " " << _right_normal.z << std::endl;
        std::cout << "Left normal: " << _left_normal.x << " " << _left_normal.y << " " << _left_normal.z << std::endl;


        return true;
    }
};



class Camera {
    public:
        static glm::mat4 perspective(float fov_y, float ratio, float z_near);

        Camera();

        void set_view(const glm::mat4& matrix);
        void set_proj(const glm::mat4& matrix);

        void set_fov(float fov);
        void set_ratio(float ratio);

        glm::vec3 position() const;
        glm::vec3 forward() const;
        glm::vec3 right() const;
        glm::vec3 up() const;

        const glm::mat4& projection_matrix() const;
        const glm::mat4& view_matrix() const;
        const glm::mat4& view_proj_matrix() const;

        bool is_orthographic() const;

        float fov() const;
        float ratio() const;

        Frustum build_frustum() const;

    private:
        void update();

        glm::mat4 _projection;
        glm::mat4 _view;
        glm::mat4 _view_proj;
};

}

#endif // CAMERA_H
