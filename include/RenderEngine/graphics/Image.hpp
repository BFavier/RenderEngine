# pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/graphics/ImageFormat.hpp>
#include <RenderEngine/graphics/Buffer.hpp>
#include <RenderEngine/graphics/AntiAliasing.hpp>
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
    public: // This class is non copyable
        Image() = delete;
        Image(const Image& other) = delete;
        Image& operator=(const Image& other) = delete;
    public:
        // Load an image from file
        Image(const std::shared_ptr<GPU>& gpu, const std::string& file_path,
              std::optional<ImageFormat> format = {},
              bool texture_compatible = false, bool storage_compatible = false,
              VkMemoryPropertyFlags memory_type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              AntiAliasing sample_count = AntiAliasing::X1);
        // create an image from dimensions and format
        Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, uint32_t layers,
              ImageFormat format,
              bool texture_compatible = false, bool storage_compatible = false,
              VkMemoryPropertyFlags memory_type = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
              AntiAliasing sample_count = AntiAliasing::X1);
        // Create an image view from an existing VkImage
        Image(const std::shared_ptr<GPU>& gpu, const VkImage& vk_image, uint32_t width, uint32_t height,
              ImageFormat format, bool texture_compatible = false, bool storage_compatible = false, AntiAliasing sample_count = AntiAliasing::X1);
        // Image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, ImageFormat format, const std::vector<unsigned char>& data);
        ~Image();
    public:
        std::shared_ptr<GPU> gpu;
    protected:
        VkImage _vk_image = VK_NULL_HANDLE;
        VkImageView _vk_image_view = VK_NULL_HANDLE;
        VkSampler _vk_sampler = VK_NULL_HANDLE;
        VkDeviceMemory _vk_device_memory = VK_NULL_HANDLE;
        uint32_t _width;
        uint32_t _height;
        uint32_t _layers;
        ImageFormat _format;
        VkImageLayout _current_layout; // the current layout of the image in memory
        std::optional<std::tuple<uint32_t, VkQueue, VkCommandPool>> _current_queue; // The (queue family index, queue, command pool) the image is currently used by, if any.
        bool _texture_compatible;
        bool _storage_compatible;
        bool _owned_vk_image;
        AntiAliasing _sample_count;
        uint32_t _mip_levels;
    protected:
        void _set_attributes(uint32_t width, uint32_t height, uint32_t layers, ImageFormat format, bool texture_compatible, bool storage_compatible, AntiAliasing sample_count);
        void _allocate_vk_image(uint32_t width, uint32_t height, uint32_t layers, ImageFormat format, bool texture_compatible, bool storage_compatible, VkMemoryPropertyFlags memory_type, AntiAliasing sample_count);
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
        uint32_t layers() const;
        ImageFormat format() const;
        uint32_t channel_count() const;
        size_t channel_bytes_size() const;
        AntiAliasing sample_count() const;
        bool is_texture_compatible() const;
        void save_to_disk(const std::string& file_path);
        void upload_data(const std::vector<uint8_t>& pixels, uint32_t layer_offset=0);
        std::vector<uint8_t> download_data(uint32_t layer=0);
    public:
        static uint8_t format_channel_count(const ImageFormat& format);
        static size_t format_channel_bytessize(const ImageFormat& format);
        static std::tuple<std::vector<uint8_t>, uint32_t, uint32_t, ImageFormat> read_pixels_from_file(const std::string& file_path, const std::optional<ImageFormat>& requested_format = {});
        static void save_pixels_to_file(const std::string& file_path, const std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height, ImageFormat format);
    };
}
