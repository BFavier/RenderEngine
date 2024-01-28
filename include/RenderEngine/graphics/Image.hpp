# pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <string>
#include <memory>
#include <optional>
#include <functional>

namespace RenderEngine
{
    class ImageView;

    class Image
    // An image is a collection of pixels stored on a gpu, with a width, a height, and a color format
    {
    public:
        enum Format {GRAY=VK_FORMAT_R8_SRGB, RGB=VK_FORMAT_B8G8R8_SRGB, RGBA=VK_FORMAT_B8G8R8A8_SRGB};
    public:
        Image() = delete;
        Image(const GPU& gpu, const std::string& file_path, std::optional<Image::Format> = std::optional<Image::Format>());
        Image(const GPU& gpu, uint32_t width, uint32_t height, Image::Format format);
        Image(const GPU& gpu, uint32_t width, uint32_t height, Image::Format format, const VkImage& vk_image);
        Image(const GPU& gpu, uint32_t width, uint32_t height, Image::Format format, const std::vector<unsigned char>& data);
        ~Image();
    public:
        const GPU& gpu;
        uint32_t width;
        uint32_t height;
        Format format;
        VkImage _vk_image = VK_NULL_HANDLE;
        VkImageView _vk_image_view = VK_NULL_HANDLE;
    protected:
        void allocate_vk_image(const std::optional<VkImage>& image = std::optional<VkImage>());
        void allocate_vk_image_view();
        // void upload_data(unsigned char* data);
    };
}