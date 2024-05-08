# pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/graphics/ImageFormat.hpp>
#include <RenderEngine/graphics/Buffer.hpp>
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
              std::optional<ImageFormat> format = {}, Image::AntiAliasing sample_count = Image::AntiAliasing::X1,
              bool texture_compatible = false, bool storage_compatible = false,
              VkMemoryPropertyFlags memory_type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // create an image from dimensions and format
        Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, ImageFormat format,
              Image::AntiAliasing sample_count = Image::AntiAliasing::X1,
              bool texture_compatible = false, bool storage_compatible = false,
              VkMemoryPropertyFlags memory_type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        // Create an image view from an existing VkImage
        Image(const std::shared_ptr<GPU>& gpu, const std::shared_ptr<VkImage>& vk_image, uint32_t width, uint32_t height, ImageFormat format,
              Image::AntiAliasing sample_count = Image::AntiAliasing::X1,
              bool texture_compatible = false, bool storage_compatible = false);
        // Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, ImageFormat format, const std::vector<unsigned char>& data);
        ~Image();
    public:
        std::shared_ptr<GPU> gpu;
        std::shared_ptr<Buffer> staging_buffer = nullptr;
        std::shared_ptr<VkImage> _vk_image = nullptr;
        std::shared_ptr<VkImageView> _vk_image_view = nullptr;
        std::shared_ptr<VkSampler> _vk_sampler = nullptr;
        std::shared_ptr<VkDeviceMemory> _vk_device_memory = nullptr;
    protected:
        uint32_t _width;
        uint32_t _height;
        ImageFormat _format;
        VkImageLayout _layout; // the current layout of the image ()
        std::optional<std::tuple<uint32_t, VkQueue, VkCommandPool>> _current_queue = {}; // The (queue index, queue, command pool) the image is currently used by, if any.
        Image::AntiAliasing _sample_count;
        bool _texture_compatible;
        uint32_t _mip_levels;
    protected:
        void _set_attributes(uint32_t width, uint32_t height, ImageFormat format, Image::AntiAliasing sample_count, bool texture_compatible, bool storage_compatible);
        void _allocate_vk_image(uint32_t width, uint32_t height, ImageFormat format, Image::AntiAliasing sample_count, bool texture_compatible, bool storage_compatible, VkMemoryPropertyFlags memory_type);
        void _allocate_vk_image_view();
        uint32_t _find_memory_type_index(uint32_t memory_type_bits, VkMemoryPropertyFlags _memory_type) const;
        void _transition_to_layout(VkImageLayout new_layout, VkCommandBuffer command_buffer);
        void _fill_layout_attributes(VkImageLayout new_layout, uint32_t& queue_family_index, VkAccessFlags& acces_mask, VkPipelineStageFlags& stage) const;
        VkImageAspectFlags _get_aspect_mask() const;
        VkCommandBuffer _begin_single_time_commands();
        void _end_single_time_commands(VkCommandBuffer commandBuffer);
    public:
        uint32_t width() const;
        uint32_t height() const;
        ImageFormat format() const;
        uint32_t channel_count() const;
        uint32_t channel_bytes_size() const;
        AntiAliasing sample_count() const;
        bool is_texture_compatible() const;
        void save_to_disk(const std::string& file_path);
        void upload_data(const std::vector<uint32_t>& pixels);
        std::vector<uint32_t> download_data();
    public:
        static uint8_t format_channel_count(const ImageFormat& format);
        static size_t format_channel_bytessize(const ImageFormat& format);
        static std::tuple<std::vector<uint8_t>, uint32_t, uint32_t, ImageFormat> read_pixels_from_file(const std::string& file_path, const std::optional<ImageFormat>& requested_format = {});
        static void save_pixels_to_file(const std::string& file_path, const std::vector<uint32_t>& pixels, uint32_t& width, uint32_t& height, ImageFormat format);
        static void _deallocate_image(const std::shared_ptr<GPU>& gpu, VkImage* vk_image);
        static void _deallocate_image_view(const std::shared_ptr<GPU>& gpu, VkImageView* vk_image_view);
        static void _free_image_memory(const std::shared_ptr<GPU>& gpu, VkDeviceMemory* vk_device_memory);
        static void _deallocate_sampler(const std::shared_ptr<GPU>& gpu, VkSampler* sampler);
    };
}
