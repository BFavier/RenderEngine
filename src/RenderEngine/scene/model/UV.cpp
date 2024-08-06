#include <RenderEngine/scene/model/UV.hpp>
using namespace RenderEngine;

UV::UV()
{
}

UV::UV(float _u, float _v)
{
    u = _u;
    v = _v;
}

UV::~UV()
{
}

vec2 UV::to_vec() const
{
    return vec2({u, v});
}