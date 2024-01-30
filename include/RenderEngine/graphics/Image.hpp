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

    class Image
    // An image is a collection of pixels stored on a gpu, with a width, a height, and a color format
    {
    public:
        enum Format {GRAY=VK_FORMAT_R8_SRGB, RGB=VK_FORMAT_B8G8R8_SRGB, RGBA=VK_FORMAT_B8G8R8A8_SRGB};
    public:
        Image() = delete;
        Image(const std::shared_ptr<GPU>& gpu, const std::string& file_path, std::optional<Image::Format> = std::optional<Image::Format>());
        Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, Image::Format format);
        Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, Image::Format format, const std::shared_ptr<VkImage>& vk_image);
        Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, Image::Format format, const std::vector<unsigned char>& data);
        ~Image();
    public:
        std::shared_ptr<GPU> gpu;
        uint32_t width;
        uint32_t height;
        Format format;
    protected:
        std::shared_ptr<VkImage> _vk_image = nullptr;
        std::shared_ptr<VkImageView> _vk_image_view = nullptr;
    protected:
        void allocate_vk_image();
        void allocate_vk_image_view();
        // void upload_data(unsigned char* data);
    public:
        static void _deallocate_image(const std::shared_ptr<GPU>& gpu, VkImage* vk_image);
        static void _deallocate_image_view(const std::shared_ptr<GPU>& gpu, VkImageView* vk_image_view);
    };
}