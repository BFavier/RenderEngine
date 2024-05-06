#pragma once
#include <RenderEngine/graphics/Canvas.hpp>
#include <RenderEngine/graphics/Image.hpp>
#include <RenderEngine/graphics/Light.hpp>

namespace RenderEngine
{
    class ShadowMap
    {
        public:
            ShadowMap() = delete;
            ShadowMap(const Light& light, const Canvas& canvas);
            ~ShadowMap();
        public:
            Image shadow;
    };
}
