#pragma once
#include <RenderEngine/utilities/External.hpp>


namespace RenderEngine
{
    class Canvas
    // A canvas is an image that can be drawn onto. A Canvas image is necessarly in RGBA format.
    {
        public:
            Canvas();
            ~Canvas();
        protected:
            void draw();
    };
}