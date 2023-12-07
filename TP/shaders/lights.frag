#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_depth;
layout(binding = 1) uniform sampler2D in_albedo_roughness;
layout(binding = 2) uniform sampler2D in_normal_metallic;


layout(binding = 0) uniform Data {
    FrameData frame;
};

vec3 unproject(vec2 uv, float depth, mat4 inv_viewproj) {
    const vec3 ndc = vec3(uv * 2.0 - vec2(1.0), depth);
    const vec4 p = inv_viewproj * vec4(ndc, 1.0);
    return p.xyz / p.w;
}

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    float depth = texelFetch(in_depth, coord, 0).r;
    vec3 albedo = texelFetch(in_albedo_roughness, coord, 0).rgb;
    vec3 normals = texelFetch(in_normal_metallic, coord, 0).rgb;

    mat4 cam_matrix = inverse(frame.camera.view_proj);

    vec3 hdr = unproject(in_uv, depth, cam_matrix);

    vec3 color = albedo;

    out_color = vec4(color, 1.0);
}