#version 450

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_material;

layout(location = 0) out vec4 color;
layout(location = 1) out vec4 normal_SNORM;
layout(location = 2) out vec4 material_UNORM;

void main()
{
    color = frag_color;
    normal_SNORM = vec4(frag_normal, 1.0);
    material_UNORM = vec4(frag_material, 1.0);
}