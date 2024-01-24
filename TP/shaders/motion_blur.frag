#version 450

layout(binding = 0) uniform sampler2D in_depth;
layout(binding = 1) uniform sampler2D in_albedo_roughness;
layout(binding = 2) uniform sampler2D in_normal_metallic;
layout(binding = 3) uniform sampler2D motion_vector;

out vec4 FragColor;

void main()
{
    const ivec2 coord = ivec2(gl_FragCoord.xy);

    vec2 MotionVector = texture(motion_vector, coord).xy / 2.0;

    vec4 Color = vec4(0.0);

    vec2 TexCoord = vec2(coord);

    Color += texture(in_albedo_roughness, coord) * 0.4;
    TexCoord -= MotionVector;
    Color += texture(in_albedo_roughness, coord) * 0.3;
    TexCoord -= MotionVector;
    Color += texture(in_albedo_roughness, coord) * 0.2;
    TexCoord -= MotionVector;
    Color += texture(in_albedo_roughness, coord) * 0.1;

    FragColor = Color;
}