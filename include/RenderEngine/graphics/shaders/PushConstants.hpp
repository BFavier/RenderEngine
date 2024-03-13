#pragma once
#include <RenderEngine/graphics/shaders/Type.hpp>

namespace RenderEngine
{
    #pragma pack(push, 1)  // removes padding in binary representation from structs defined below
    struct PushConstants
    {
        mat3 mesh_rotation;
    };
    #pragma pack(pop)
}