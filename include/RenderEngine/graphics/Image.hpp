# pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/graphics/Format.hpp>
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
        enum AntiAliasing {X1=VK_SAMPLE_COUNT_1_BIT,
                           X2=VK_SAMPLE_COUNT_2_BIT,
                           X4=VK_SAMPLE_COUNT_4_BIT,
                           X16=VK_SAMPLE_COUNT_16_BIT,
                           X32=VK_SAMPLE_COUNT_32_BIT,
                           X64=VK_SAMPLE_COUNT_64_BIT};
    public:
        Image() = delete;
        Image(const std::shared_ptr<GPU>& gpu, const std::string& file_path, std::optional<Format> = std::optional<Format>());
        Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, Format format);
        Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, Format format, Image::AntiAliasing sample_count, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlagBits memory_type, bool mip_mapping);
        Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, Format format, const std::shared_ptr<VkImage>& vk_image);
        // Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, Format format, const std::vector<unsigned char>& data);
        ~Image();
    public:
        std::shared_ptr<GPU> gpu;
        std::shared_ptr<VkImage> _vk_image = nullptr;
        std::shared_ptr<VkImageView> _vk_image_view = nullptr;
    protected:
        uint32_t _width;
        uint32_t _height;
        Format _format;
        VkImageLayout _layout;
        VkDeviceMemory _memory;
        AntiAliasing _sample_count = X1;
        VkImageTiling _tiling = VK_IMAGE_TILING_OPTIMAL;
        VkImageUsageFlags _usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        VkMemoryPropertyFlagBits _memory_type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        uint32_t _mip_levels = 1;
    protected:
        void allocate_vk_image();
        void allocate_vk_image_view();
        uint32_t _find_memory_type_index(uint32_t memory_type_bits, VkMemoryPropertyFlagBits _memory_type) const;
        void _transition_to_layout(VkImageLayout new_layout, VkCommandBuffer command_buffer);
        // void upload_data(unsigned char* data);
    public:
        uint32_t width() const;
        uint32_t height() const;
        Format format() const;
        AntiAliasing sample_count() const;
        static void _deallocate_image(const std::shared_ptr<GPU>& gpu, VkImage* vk_image);
        static void _deallocate_image_view(const std::shared_ptr<GPU>& gpu, VkImageView* vk_image_view);
    };
}