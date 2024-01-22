#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <memory>

namespace RenderEngine
{
    class Image;

    class ImageView
    {
    public:
        ImageView() = delete;
        ImageView(const Image& image, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
        ~ImageView();
    public:
        const Image& image;
        VkImageView _vk_image_view;
        VkPipelineLayout _vk_graphic_pipeline;
    protected:
        void _set_vk_image_view();
        void _set_graphic_pipeline();
    };
}
