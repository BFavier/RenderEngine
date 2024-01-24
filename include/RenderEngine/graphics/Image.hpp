# pragma once
#include <RenderEngine/utilities/External.hpp>
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
        Image(const Image& other);
        ~Image();
    public:
        void operator=(const Image& other);
    public:
        const GPU* gpu;
        uint32_t width;
        uint32_t height;
        Format format;
        std::shared_ptr<VkImage> _vk_image;
        std::shared_ptr<VkImageView> _vk_image_view;
    public:
        static void _deallocate_vk_image(VkImage* vk_image, const GPU& gpu);
        static void _deallocate_vk_image_view(VkImageView* vk_image_view, const GPU& gpu);
    protected:
        void allocate_vk_image(const std::optional<VkImage>& image = std::optional<VkImage>());
        void allocate_vk_image_view();
        void upload_data(unsigned char* data);
    };
}