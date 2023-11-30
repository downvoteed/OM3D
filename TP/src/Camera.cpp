#include "Camera.h"

namespace OM3D {


static bool is_proj_orthographic(const glm::mat4& proj) {
    return proj[3][3] == 1.0f;
}

static float extract_ratio(const glm::mat4& proj) {
    const float f = proj[1][1];
    return std::abs(1.0f / (proj[0][0] / f));
}

static float extract_near(const glm::mat4& proj) {
    return proj[3][2];
}

static float extract_fov(const glm::mat4& proj) {
    ALWAYS_ASSERT(!is_proj_orthographic(proj), "Orthographic camera doesn't have a FoV");
    const float f = proj[1][1];
    return 2.0f * std::atan(1.0f / f);
}

static glm::vec3 extract_position(const glm::mat4& view) {
    glm::vec3 pos = {};
    for(u32 i = 0; i != 3; ++i) {
        pos -= glm::vec3(view[0][i], view[1][i], view[2][i]) * view[3][i];
    }
    return pos;
}

static glm::vec3 extract_forward(const glm::mat4& view) {
    return -glm::normalize(glm::vec3(view[0][2], view[1][2], view[2][2]));
}

static glm::vec3 extract_right(const glm::mat4& view) {
    return glm::normalize(glm::vec3(view[0][0], view[1][0], view[2][0]));
}

static glm::vec3 extract_up(const glm::mat4& view) {
    return glm::normalize(glm::vec3(view[0][1], view[1][1], view[2][1]));
}





glm::mat4 Camera::perspective(float fov_y, float ratio, float z_near) {
    float f = 1.0f / std::tan(fov_y / 2.0f);
    return glm::mat4(f / ratio, 0.0f,  0.0f,  0.0f,
                  0.0f,    f,  0.0f,  0.0f,
                  0.0f, 0.0f,  0.0f, -1.0f,
                  0.0f, 0.0f, z_near,  0.0f);
}

Camera::Camera() {
    _projection = perspective(to_rad(60.0f), 16.0f / 9.0f, 0.001f);
    //_view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.0f, 4.0f, 0.0f));
    // pour la foret
    _view = glm::lookAt(glm::vec3(200.0f, 300.0f, 300.0f), // Augmente la hauteur et recule la caméra
                    glm::vec3(0.f, 0.f, 0.f),    // Point de cible inchangé
                    glm::vec3(0.0f, 4.0f, 0.0f));// Vecteur vers le haut inchangé


    update();
}

void Camera::set_view(const glm::mat4& matrix) {
    _view = matrix;
    //std::cout << "Camera position: " << extract_position(_view).x << " " << extract_position(_view).y << " " << extract_position(_view).z << std::endl;
    update();
}

void Camera::set_proj(const glm::mat4& matrix) {
    _projection = matrix;
    update();
}

void Camera::set_fov(float fov) {
    set_proj(perspective(fov, ratio(), extract_near(_projection)));
}

void Camera::set_ratio(float ratio) {
    set_proj(perspective(fov(), ratio, extract_near(_projection)));
}

glm::vec3 Camera::position() const {
    return extract_position(_view);
}

glm::vec3 Camera::forward() const {
    return extract_forward(_view);
}

glm::vec3 Camera::right() const {
    return extract_right(_view);
}

glm::vec3 Camera::up() const {
    return extract_up(_view);
}

const glm::mat4& Camera::projection_matrix() const {
    return _projection;
}

const glm::mat4& Camera::view_matrix() const {
    return _view;
}

const glm::mat4& Camera::view_proj_matrix() const {
    return _view_proj;
}

bool Camera::is_orthographic() const {
    return is_proj_orthographic(_projection);
}

float Camera::fov() const {
    return is_orthographic() ? 0.0f : extract_fov(_projection);
}

float Camera::ratio() const {
    return extract_ratio(_projection);
}

void Camera::update() {
    _view_proj = _projection * _view;
}

Frustum Camera::build_frustum() const {
    const glm::vec3 camera_forward = forward();
    const glm::vec3 camera_up = up();
    const glm::vec3 camera_right = right();

    Frustum frustum;
    frustum._near_normal = camera_forward;

    const float half_fov = fov() * 0.5f;
    const float half_fov_v = std::atan(std::tan(half_fov) * ratio());
    {
        const float c = std::cos(half_fov);
        const float s = std::sin(half_fov);
        frustum._bottom_normal = camera_forward * s + camera_up * c;
        frustum._top_normal = camera_forward * s - camera_up * c;
    }
    {
        const float c = std::cos(half_fov_v);
        const float s = std::sin(half_fov_v);
        frustum._left_normal = camera_forward * s + camera_right * c;
        frustum._right_normal = camera_forward * s - camera_right * c;
    }

    return frustum;
}
bool Camera::isInside(const SceneObject& obj, const Frustum& frustum) const
{
    float radius = obj.get_bounding_sphere().radius();
    glm::vec4 test = obj.transform() * glm::vec4(obj.get_bounding_sphere().center().position, 1.0f);
    glm::vec3 sphere_position = glm::vec3(test.x, test.y, test.z);

    if (dot(frustum._top_normal, sphere_position - this->position()) < -radius)
        return false;
    
    if (dot(frustum._bottom_normal, sphere_position - this->position()) < -radius)
        return false;

    if (dot(frustum._right_normal, sphere_position - this->position()) < -radius)
        return false;

    if (dot(frustum._left_normal, sphere_position - this->position()) < -radius)
        return false;

    if (dot(frustum._near_normal, sphere_position - (this->position() + this->forward())) < -radius)
        return false;

    return true;
}

}
