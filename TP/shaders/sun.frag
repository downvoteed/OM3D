#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_albedo_roughness;
layout(binding = 1) uniform sampler2D in_normal_metallic;

layout(binding = 0) uniform Data {
    FrameData frame;
};


void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    vec3 albedo = texelFetch(in_albedo_roughness, coord, 0).rgb;
    vec3 normals = texelFetch(in_normal_metallic, coord, 0).rgb;

    vec3 normalizedNormal = normalize(normals);

    vec3 ambient = vec3(0.f, 0.f, 0.f);

    vec3 acc = frame.sun_color * max(0.0, dot(frame.sun_dir, normalizedNormal)) + ambient;

    vec3 color = albedo * acc * 2.0;

    out_color = vec4(color, 1.0);
}