//#version 450
//
//#include "utils.glsl"
//
//layout(location = 0) out vec4 out_color;
//
//layout(location = 0) in vec2 in_uv;
//
//layout(binding = 0) uniform sampler2D in_depth;
//layout(binding = 1) uniform sampler2D in_albedo_roughness;
//layout(binding = 2) uniform sampler2D in_normal_metallic;
//
//layout(binding = 0) uniform Data {
//    FrameData frame;
//};
//
//vec3 unproject(vec2 uv, float depth, mat4 inv_viewproj) {
//    const vec3 ndc = vec3(uv * 2.0 - vec2(1.0), depth);
//    const vec4 p = inv_viewproj * vec4(ndc, 1.0);
//    return p.xyz / p.w;
//}
//
//void main() {
//    const ivec2 coord = ivec2(gl_FragCoord.xy);
//
//    float depth = texelFetch(in_depth, coord, 0).r;
//    vec3 albedo = texelFetch(in_albedo_roughness, coord, 0).rgb;
//    vec3 normals = texelFetch(in_normal_metallic, coord, 0).rgb;
//
//    mat4 cam_matrix = inverse(frame.camera.view_proj);
//
//    vec3 hdr = unproject(in_uv, depth, cam_matrix);
//
//    vec3 color = albedo;
//
//    out_color = vec4(color, 1.0);
//}

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

layout(binding = 1) buffer PointLights {
    PointLight point_lights[];
};

vec3 unproject(vec2 uv, float depth, mat4 inv_viewproj) {
    vec4 clipSpacePosition = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 viewSpacePosition = inv_viewproj * clipSpacePosition;
    return viewSpacePosition.xyz / viewSpacePosition.w;
}

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);
    float depth = texelFetch(in_depth, coord, 0).r;
    vec3 albedo = texelFetch(in_albedo_roughness, coord, 0).rgb;
    vec3 normal = texelFetch(in_normal_metallic, coord, 0).rgb;
    vec3 acc = vec3(0.0);

    mat4 invViewProj = inverse(frame.camera.view_proj);
    vec3 position = unproject(in_uv, depth, invViewProj);

    for (uint i = 0; i != frame.point_light_count; ++i) {
        PointLight light = point_lights[i];
        const vec3 to_light = (light.position - position);
        const float dist = length(to_light);
        const vec3 light_vec = to_light / dist;
        const float NoL = dot(light_vec, normal);
        const float att = attenuation(dist, light.radius);
        if (NoL > 0.0 && att > 0.0) {
            acc += light.color * (NoL * att);
        }
    }

    vec3 color = albedo * acc;
    out_color = vec4(color, 1.0);
}
