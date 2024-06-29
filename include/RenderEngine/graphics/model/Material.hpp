#pragma once
#include <RenderEngine/graphics/shaders/Types.hpp>

namespace RenderEngine
{
    class Material
    {
    public:
        Material();
        Material(double _rougness, double _metalness, double _luminance);
        ~Material();
    public:
        double metalness = 0.;
        double roughness = 0.;
        double luminance = 0.;
    public:
        vec3 to_vec() const;
    };
}