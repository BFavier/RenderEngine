# pragma once
#include <RenderEngine/utilities/External.hpp>
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
        friend class SwapChain;
        friend class Canvas;
        friend class Window;
    public: // This class is non copyable
        Image() = delete;
        Image(const Image& other) = delete;
        Image& operator=(const Image& other) = delete;
    public:
        // Load an image from file
        Image(const std::shared_ptr<GPU>& gpu, const std::string& file_path,
              ImageFormat format,
              const std::optional<uint32_t>& resized_width=std::nullopt,
              const std::optional<uint32_t>& resized_height=std::nullopt);
        // create an image from dimensions and format
        Image(const std::shared_ptr<GPU>& gpu, ImageFormat format, uint32_t width, uint32_t height, bool mipmaped=true, AntiAliasing sample_count=AntiAliasing::X1);
        // Create an image view from an existing VkImage, if vk_device_memory==nullptr, the vk_image won't be destroyed by Image destructor (it is understood to be owned by the swapchain)
        Image(const std::shared_ptr<GPU>& gpu, const VkImage& vk_image, const std::shared_ptr<VkDeviceMemory>& vk_device_memory,
              ImageFormat format, uint32_t width, uint32_t height, bool mipmaped);
        ~Image();
    protected:
        std::shared_ptr<GPU> _gpu = nullptr;
        VkImage _vk_image = VK_NULL_HANDLE;
        std::shared_ptr<VkDeviceMemory> _vk_device_memory = nullptr;  // this is not used, only keeping a reference to avoid premature deallocation
        VkImageView _vk_image_view = VK_NULL_HANDLE;
        VkSampler _vk_sampler = VK_NULL_HANDLE;
        uint32_t _width;
        uint32_t _height;
        uint32_t _mip_levels;
        ImageFormat _format;
        VkImageLayout _current_layout = VK_IMAGE_LAYOUT_UNDEFINED; // the current layout of the image in memory
        std::optional<std::tuple<uint32_t, VkQueue, VkCommandPool>> _current_queue = std::nullopt; // The (queue family index, queue, command pool) the image is currently used by, if any.
    protected:
        // Create a 'vk_image' image from the given properties
        static VkImage _create_vk_image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, ImageFormat format, uint32_t mip_levels, AntiAliasing sample_count);
        // Allocate an array of memory in which 'n_images' images similar to 'vk_image' can be stored. Returns the vk_device_memory and offset in memory between two images.
        static std::tuple<std::shared_ptr<VkDeviceMemory>, std::size_t> _allocate_vk_device_memory(const std::shared_ptr<GPU>& gpu, const VkImage& vk_image, uint32_t n_images);
        // binds the given 'vk_image' to the given offset of a 'vk_device_memory' memory array.
        static void _bind_image_to_memory(const std::shared_ptr<GPU>& gpu, const VkImage& vk_image, const std::shared_ptr<VkDeviceMemory>& vk_device_memory, std::size_t offset);
        // return the mip levels count for an image of given width/height
        static uint32_t _mip_levels_count(uint32_t width, uint32_t height);
        // returns the source layout attributes for a given layout transition
        static std::tuple<VkAccessFlagBits, VkPipelineStageFlagBits> _source_layout_attributes(VkImageLayout layout);
        // returns the destination layout attributes for a given layout transition
        static std::tuple<VkAccessFlagBits, VkPipelineStageFlagBits> _destination_layout_attributes(VkImageLayout layout);
        void _create_vk_sampler();
        void _create_vk_image_view();
        VkImageAspectFlags _get_aspect_mask() const;
        VkCommandBuffer _begin_single_time_commands();
        void _end_single_time_commands(VkCommandBuffer commandBuffer);
    public:
        uint32_t width() const;
        uint32_t height() const;
        ImageFormat format() const;
        uint32_t mip_levels_count() const;
        void save_to_disk(const std::string& file_path);
        void upload_data(const std::vector<uint8_t>& pixels);
        std::vector<uint8_t> download_data();
    public:
        static std::vector<std::shared_ptr<Image>> bulk_allocate_images(const std::shared_ptr<GPU>& gpu, uint32_t n_images, ImageFormat format, uint32_t width, uint32_t height, bool mipmaped=true);
        static std::vector<std::shared_ptr<Image>> bulk_load_images(const std::shared_ptr<GPU>& gpu, const std::vector<std::string>& file_paths, ImageFormat format, uint32_t width, uint32_t height, bool mipmaped=true);
        static std::map<std::string, std::shared_ptr<Image>> bulk_load_images(const std::shared_ptr<GPU>& gpu, const std::map<std::string, std::string>& file_paths, ImageFormat format, uint32_t width, uint32_t height, bool mipmaped=true);
        static std::tuple<std::vector<uint8_t>, uint32_t, uint32_t> read_pixels_from_file(const std::string& file_path);
        static std::tuple<std::vector<float>, uint32_t, uint32_t> read_float_pixels_from_file(const std::string& file_path);
        static std::vector<uint8_t> resize_pixels(const std::vector<uint8_t>& pixels, const std::pair<uint32_t, uint32_t>& width_height, const std::pair<uint32_t, uint32_t>& new_width_height);
        static std::vector<float> resize_float_pixels(const std::vector<float>& pixels, const std::pair<uint32_t, uint32_t>& width_height, const std::pair<uint32_t, uint32_t>& new_width_height);
        static void save_pixels_to_file(const std::string& file_path, const std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height);
        static void save_float_pixels_to_file(const std::string& file_path, const std::vector<float>& pixels, uint32_t& width, uint32_t& height);
    };
}
