#pragma once
#include <RenderEngine/graphics/shaders/Types.hpp>

namespace RenderEngine
{
    struct Vertex
    {
        vec3 position;  // x, y, z
        vec3 normal;  // nx, ny, nz
        vec4 color;  // r, g, b, a
    };
}