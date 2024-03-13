#pragma once
#include <RenderEngine/graphics/shaders/Type.hpp>

namespace RenderEngine
{
    // sending vec3 to shader requires padding of structs that is not consistent across various drivers
    #pragma pack(push, 1)  // removes padding in binary representation from structs defined below
    struct PushConstants
    {
        vec4 mesh_position;
        vec4 mesh_scale;
        mat3 mesh_rotation;
    };
    #pragma pack(pop)
}