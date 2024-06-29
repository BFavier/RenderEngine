#include <RenderEngine/graphics/Color.hpp>
#include <RenderEngine/utilities/Macro.hpp>
using namespace RenderEngine;

Color::Color()
{
}

Color::Color(float _r, float _g, float _b, float _a)
{
    r = _r;
    g = _g;
    b = _b;
    a = _a;
    if (r < 0.0 || r > 1.0 || g < 0.0 || g > 1.0 || b < 0.0 || b > 1.0 || a < 0.0 || a > 1.0)
    {
        THROW_ERROR("Color object expects all values to be between 0.0 and 1.0")
    }
}

Color::~Color()
{
}

vec4 Color::to_vec4() const
{
    return vec4({r, g, b, a});
}
