#version 450
// RenderEngine.clear_on_load = true
// RenderEngine.blending = Blending::OVERWRITE

layout(location = 0) out vec4 albedo;
layout(location = 1) out vec4 normal_SNORM;
layout(location = 2) out vec4 material_UNORM;
layout(location = 3) out vec4 color;

void main()
{
    albedo = vec4(0.);
    normal_SNORM = vec4(0.);
    material_UNORM = vec4(0.);
    color = vec4(0.);
}