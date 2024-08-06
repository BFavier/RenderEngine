#include <RenderEngine/scene/model/UV.hpp>
using namespace RenderEngine;

UV::UV()
{
}

UV::UV(double _u, double _v)
{
    u = _u;
    v = _v;
}

UV::~UV()
{
}

vec2 UV::to_vec() const
{
    return vec2({static_cast<float>(u), static_cast<float>(v)});
}