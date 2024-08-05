#pragma once
#include <RenderEngine/graphics/shaders/Types.hpp>

namespace RenderEngine
{
    class Color
    {
    public:
        Color();
        Color(float _r, float _g, float _b, float _a=1.0);
        ~Color();
    public:
        float r = 1.0;
        float g = 1.0;
        float b = 1.0;
        float a = 1.0;
    public:
        vec4 to_vec4() const;
    };
}