#version 450

#include "utils.glsl"

layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D in_albedo_roughness;
layout(binding = 1) uniform sampler2D in_normal_metallic;
layout(binding = 2) uniform sampler2D in_depth;

float reinhard(float hdr) {
    return hdr / (hdr + 1.0);
}

vec3 reinhard(vec3 x) {
    return vec3(reinhard(x.x), reinhard(x.y), reinhard(x.z));
}

void main() {
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    const vec3 hdr = texelFetch(in_albedo_roughness, coord, 0).rgb;
    //const vec3 tone_mapped = reinhard(hdr);

    out_color = vec4(hdr, 1.0);
}
