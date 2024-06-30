#pragma once
#include <RenderEngine/graphics/Camera.hpp>
#include <RenderEngine/graphics/Color.hpp>

namespace RenderEngine
{
    class Light : public Camera
    {
        public:
            Light();
            ~Light();
        public:
            Color color;
            double luminance = 0;

    };
}
