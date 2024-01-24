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
layout(location = 6) out vec4 out_clip_space_position;
layout(location = 7) out vec4 out_clip_space_prev_position;

layout(binding = 0) uniform Data {
    FrameData frame;
};

uniform mat4 model;
uniform bool u_has_skeleton;

const int MAX_JOINTS = 256;

uniform mat4 u_joint_matrix[MAX_JOINTS];
uniform mat4 u_joint_matrix_prev[MAX_JOINTS];

void main()
{
    out_normal = normalize(mat3(model) * in_normal);
    out_tangent = normalize(mat3(model) * in_tangent_bitangent_sign.xyz);
    out_bitangent = cross(out_tangent, out_normal) * (in_tangent_bitangent_sign.w > 0.0 ? 1.0 : -1.0);

    out_uv = in_uv;
    out_color = in_color;

    vec4 position = vec4(vec3(0), 1);

    if (u_has_skeleton)
    {
        mat4 skin_matrix =
        in_weight.x * u_joint_matrix[int(in_joint.x)] +
        in_weight.y * u_joint_matrix[int(in_joint.y)] +
        in_weight.z * u_joint_matrix[int(in_joint.z)] +
        in_weight.w * u_joint_matrix[int(in_joint.w)];

        position = skin_matrix * vec4(in_pos, 1.0);
        out_clip_space_position = frame.camera.view_proj * model * position;

        mat4 prev_skin_matrix =
        in_weight.x * u_joint_matrix_prev[int(in_joint.x)] +
        in_weight.y * u_joint_matrix_prev[int(in_joint.y)] +
        in_weight.z * u_joint_matrix_prev[int(in_joint.z)] +
        in_weight.w * u_joint_matrix_prev[int(in_joint.w)];

        out_clip_space_prev_position = frame.camera.view_proj * model * prev_skin_matrix * vec4(in_pos, 1.0);
    }
    else
    {
        position = vec4(in_pos, 1.0);
    }

    out_position = position.xyz;
    gl_Position = frame.camera.view_proj * model * position;
}

