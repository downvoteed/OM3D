#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D in_depth;
layout(binding = 1) uniform sampler2D in_albedo_roughness;
layout(binding = 2) uniform sampler2D in_normal_metallic;

uniform bool isdepth;
uniform bool isalbedo;
uniform bool isnormal;

float reinhard(float hdr) {
    return hdr / (hdr + 1.0);
}

vec3 reinhard(vec3 x) {
    return vec3(reinhard(x.x), reinhard(x.y), reinhard(x.z));
}

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    vec3 hdr = vec3(0.0);

    if (isdepth)
        hdr = texelFetch(in_depth, coord, 0).rgb;
    if (isalbedo)
        hdr = texelFetch(in_albedo_roughness, coord, 0).rgb;
    if (isnormal)
        hdr = texelFetch(in_normal_metallic, coord, 0).rgb;

    out_color = vec4(hdr, 1.0);
}
