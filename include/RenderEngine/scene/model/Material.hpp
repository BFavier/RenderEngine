#pragma once
#include <RenderEngine/graphics/shaders/Types.hpp>

namespace RenderEngine
{
    class Material
    {
    public:
        Material();
        Material(float _rougness, float _metalness, float _ambient_occlusion);
        ~Material();
    public:
        float metalness = 0.0;
        float roughness = 0.2;
        float ambient_occlusion = 1.0;
    public:
        vec3 to_vec() const;
    };
}