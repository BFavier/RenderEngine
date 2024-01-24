# pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <string>

namespace RenderEngine
{
    class ImageView;

    class Image
    // An image is a collection of pixels stored on a gpu, with a width, a height, and a color format
    {
    public:
        enum Format {GRAY=VK_FORMAT_R8_SRGB, RGB=VK_FORMAT_R8G8B8_SRGB, RGBA=VK_FORMAT_R8G8B8A8_SRGB};
    public:
        Image() = delete;
        Image(const GPU& gpu, const std::string& file_path);
        Image(const GPU& gpu, unsigned int width, unsigned int height, Image::Format format, const std::vector<unsigned char>& data);
        ~Image();
    public:
        const GPU& gpu;
        unsigned int width;
        unsigned int height;
        Format format;
        std::shared_ptr<VkImage> _vk_image;
    protected:

    };
}