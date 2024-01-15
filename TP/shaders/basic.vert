#version 450

#include "utils.glsl"

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec4 in_tangent_bitangent_sign;
layout(location = 4) in vec3 in_color;
layout(location = 5) in vec4 in_joint;
layout(location = 6) in vec4 in_weight;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_color;
layout(location = 3) out vec3 out_position;
layout(location = 4) out vec3 out_tangent;
layout(location = 5) out vec3 out_bitangent;

layout(binding = 0) uniform Data {
    FrameData frame;
};

uniform mat4 model;

uniform mat4 u_joint_matrices[12];

void main() {
    const vec4 position = model * vec4(in_pos, 1.0);

    out_normal = normalize(mat3(model) * in_normal);
    out_tangent = normalize(mat3(model) * in_tangent_bitangent_sign.xyz);
    out_bitangent = cross(out_tangent, out_normal) * (in_tangent_bitangent_sign.w > 0.0 ? 1.0 : -1.0);

    out_uv = in_uv;
    out_color = in_color;
    
    mat4 skin_matrix = 
        in_weight.x * u_joint_matrices[int(in_joint.x)] +
        in_weight.y * u_joint_matrices[int(in_joint.y)] +
        in_weight.z * u_joint_matrices[int(in_joint.z)] +
        in_weight.w * u_joint_matrices[int(in_joint.w)];

    out_position = (skin_matrix * position).xyz;

    gl_Position = frame.camera.view_proj * skin_matrix * position;
}

