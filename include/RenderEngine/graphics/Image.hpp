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
    // An image is a collection of pixels stored on a gpu, with a width, a height, and a color format
    class Image
    {
        friend class Canvas;
        friend class Window;
    public:
        enum AntiAliasing {X1=VK_SAMPLE_COUNT_1_BIT,
                           X2=VK_SAMPLE_COUNT_2_BIT,
                           X4=VK_SAMPLE_COUNT_4_BIT,
                           X16=VK_SAMPLE_COUNT_16_BIT,
                           X32=VK_SAMPLE_COUNT_32_BIT,
                           X64=VK_SAMPLE_COUNT_64_BIT};
    public:
        Image() = delete;
        // Load an image from file
        Image(const std::shared_ptr<GPU>& gpu, const std::string& file_path,
              std::optional<Format> format = {}, Image::AntiAliasing sample_count = Image::AntiAliasing::X1, bool texture_compatible = false, VkMemoryPropertyFlags memory_type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // create an image from dimensions and format
        Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, Format format,
              Image::AntiAliasing sample_count = Image::AntiAliasing::X1, bool texture_compatible = false, VkMemoryPropertyFlags memory_type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // Create an image view from an existing image
        Image(const std::shared_ptr<GPU>& gpu, const std::shared_ptr<VkImage>& vk_image, uint32_t width, uint32_t height, Format format,
              Image::AntiAliasing sample_count = Image::AntiAliasing::X1, bool texture_compatible = false);
        // Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, Format format, const std::vector<unsigned char>& data);
        ~Image();
    public:
        std::shared_ptr<GPU> gpu;
        std::shared_ptr<VkImage> _vk_image = nullptr;
        std::shared_ptr<VkImageView> _vk_image_view = nullptr;
        std::shared_ptr<VkDeviceMemory> _vk_device_memory = nullptr;
    protected:
        uint32_t _width;
        uint32_t _height;
        Format _format;
        VkImageLayout _layout;
        Image::AntiAliasing _sample_count;
        bool _texture_compatible;
        uint32_t _mip_levels;
    protected:
        void _set_attributes(uint32_t width, uint32_t height, Format format, Image::AntiAliasing sample_count, bool texture_compatible);
        void _allocate_vk_image(uint32_t width, uint32_t height, Format format, Image::AntiAliasing sample_count, bool texture_compatible, VkMemoryPropertyFlags memory_type);
        void _allocate_vk_image_view();
        uint32_t _find_memory_type_index(uint32_t memory_type_bits, VkMemoryPropertyFlags _memory_type) const;
        void _transition_to_layout(VkImageLayout new_layout, VkCommandBuffer command_buffer);
        void _fill_layout_attributes(VkImageLayout new_layout, uint32_t& queue_family_index, VkAccessFlags& acces_mask, VkPipelineStageFlags& stage);
        VkImageAspectFlags _get_aspect_mask() const;
        // void upload_data(unsigned char* data);
    public:
        uint32_t width() const;
        uint32_t height() const;
        Format format() const;
        AntiAliasing sample_count() const;
        bool is_texture_compatible() const;
        static void _deallocate_image(const std::shared_ptr<GPU>& gpu, VkImage* vk_image);
        static void _deallocate_image_view(const std::shared_ptr<GPU>& gpu, VkImageView* vk_image_view);
        static void _free_image_memory(const std::shared_ptr<GPU>& gpu, VkDeviceMemory* vk_device_memory);
    };
}