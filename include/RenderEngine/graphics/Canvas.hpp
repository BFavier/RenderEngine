#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/Image.hpp>


namespace RenderEngine
{
    class Canvas : public Image
    // A canvas is an RGBA image that can be drawn onto.
    {
        public:
            Canvas() = delete;
            Canvas(const GPU& gpu,  uint32_t width, uint32_t height);
            Canvas(const GPU& gpu, uint32_t width, uint32_t height, const VkImage& vk_imag);
            ~Canvas();
        protected:
            void draw();
        public:
            std::shared_ptr<VkFramebuffer> _frame_buffer;
        protected:
            void allocate_frame_buffer();
        public:
            static void _deallocate_frame_buffer();
    };
}