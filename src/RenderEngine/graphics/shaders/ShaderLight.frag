#version 450

layout(binding=0) uniform sampler2D albedo;
layout(binding=1) uniform sampler2D normal;
layout(binding=2) uniform sampler2D material;

layout(location = 0) in vec2 vertex_uv;

layout(location = 0) out vec4 color;

void main()
{
    color = vec4(1.0, 0.0, 0.0, 1.0);
}