#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/Image.hpp>


namespace RenderEngine
{
    class Canvas
    // A canvas is an RGBA image that can be drawn onto.
    {
        public:
            Canvas() = delete;
            Canvas(const std::shared_ptr<GPU>& gpu,  uint32_t width, uint32_t height);
            Canvas(const Image& image);
            ~Canvas();
        public:
            std::shared_ptr<GPU> gpu = nullptr;
            Image image;
            Image handles;
            Image depth_buffer;
        protected:
            std::shared_ptr<VkFramebuffer> _frame_buffer = nullptr;
        protected:
            void allocate_frame_buffer();
        public:
            void draw();
            static void _deallocate_frame_buffer(const std::shared_ptr<GPU>& gpu, VkFramebuffer* frame_buffer);
    };
}