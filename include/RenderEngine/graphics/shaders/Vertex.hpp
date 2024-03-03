#pragma once
#include <array>

namespace RenderEngine
{
    struct Vertex
    {
        float position[3];  // x, y, z
        float normal[3];  // nx, ny, nz
        float color[4];  // r, g, b, a
    };
}