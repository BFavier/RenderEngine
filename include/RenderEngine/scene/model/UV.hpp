#pragma once
#include <RenderEngine/graphics/shaders/Types.hpp>

namespace RenderEngine
{
    class UV
    // A UV is an (x, y) normalized texture coordinates of a vertex (value between 0. and 1., with top left beeing (0., 0.))
    {
    public:
        UV();
        UV(float _u, float _v);
        ~UV();
    public:
        float u = 0.;
        float v = 0.;
    public:
        vec2 to_vec() const;
    };
}