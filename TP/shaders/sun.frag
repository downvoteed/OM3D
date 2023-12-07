#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(location = 0) in vec2 in_uv;

layout(binding = 0) uniform sampler2D in_depth;
layout(binding = 1) uniform sampler2D in_albedo_roughness;
layout(binding = 2) uniform sampler2D in_normal_metallic;


void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    float depth = texelFetch(in_depth, coord, 0).r;
    vec3 albedo = texelFetch(in_albedo_roughness, coord, 0).rgb;
    vec3 normals = texelFetch(in_normal_metallic, coord, 0).rgb;

    vec3 sunLightDirection = vec3(0, -1, -1);

    vec3 normalizedNormal = normalize(normals);

    float lightIntensity = max(dot(normalizedNormal, -sunLightDirection), 0.0);

    float distanceAttenuation = 1.0;

    float sunlightIntensity = lightIntensity * distanceAttenuation;

    vec3 color = albedo * sunlightIntensity;

    out_color = vec4(color, 1.0);
}