#version 450

layout(location = 0) in vec4 frag_color;
layout(location = 1) in vec3 frag_normal;
layout(location = 2) in vec3 frag_material;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outMaterial;

void main()
{
    outColor = frag_color;
    outNormal = vec4(frag_normal, 1.0);
    outMaterial = vec4(frag_material, 1.0);
}