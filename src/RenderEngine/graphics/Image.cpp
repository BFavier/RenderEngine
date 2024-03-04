#include <RenderEngine/graphics/Image.hpp>
#define STB_IMAGE_IMPLEMENTATION  
#include <stb/stb_image.h>
using namespace RenderEngine;

Image::Image(const std::shared_ptr<GPU>& _gpu, const std::string& image_path, std::optional<Image::Format> _format) : gpu(_gpu)
{
    int i_width, i_height, n_channels, n_required_channels;
    if (_format.has_value())
    {
        switch (_format.value())
        {
            case Image::Format::GRAY:
                n_required_channels = 1;
                break;
            case Image::Format::RGB:
                n_required_channels = 3;
                break;
            case Image::Format::RGBA:
                n_required_channels = 4;
                break;
            default:
                n_required_channels = 0;
        }
    }
    else
    {
        n_required_channels = 0;
    }
    unsigned char* imgData = stbi_load(image_path.c_str(), &i_width, &i_height, &n_channels, 0);
    //
    if (_format.has_value())
    {
        _format = _format.value();
    }
    else
    {
        _format = Image::Format::RGBA;
    }
    allocate_vk_image();
    stbi_image_free(imgData);
}

Image::Image(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height, Image::Format format) : gpu(_gpu), _width(width), _height(height), _format(format)
{
    allocate_vk_image();
    allocate_vk_image_view();
}

// Image::Image(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height, Image::Format format, const std::vector<unsigned char>& data) : gpu(_gpu), _width(width), _height(height), _format(format)
// {
//     allocate_vk_image();
//     allocate_vk_image_view();
// }

Image::Image(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height, Image::Format format, Image::AntiAliasing sample_count, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlagBits memory_type, bool mip_mapping) : gpu(_gpu), _width(width), _height(height), _format(format), _memory_type(memory_type)
{
    gpu = _gpu;
    _width = width;
    _height = height;
    _format = format;
    _sample_count = sample_count;
    _tiling = tiling;
    _usage = usage;
    if (mip_mapping)
    {
        _mip_levels = std::min(static_cast<uint32_t>(std::log2(width)), static_cast<uint32_t>(std::log2(height)));
    }
    else
    {
        _mip_levels = 1;
    }
    allocate_vk_image();
    allocate_vk_image_view();
}

Image::Image(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height, Image::Format format, const std::shared_ptr<VkImage>& vk_image) : gpu(_gpu), _width(width), _height(height), _format(format)
{
    _vk_image = vk_image;
    allocate_vk_image_view();
}

Image::~Image()
{
}

uint32_t Image::width() const
{
    return _width;
}

uint32_t Image::height() const
{
    return _height;
}

Image::Format Image::format() const
{
    return _format;
}

Image::AntiAliasing Image::sample_count() const
{
    return _sample_count;
}

void Image::allocate_vk_image()
{
    _vk_image.reset(new VkImage, std::bind(&Image::_deallocate_image, gpu, std::placeholders::_1));
    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = static_cast<VkFormat>(_format == Format::DEPTH ? gpu->depth_format().second : _format);
    info.extent.width = _width;
    info.extent.height = _height;
    info.extent.depth = 1;
    info.arrayLayers = 1; // Only one image is allocated
    info.tiling = _tiling;
    info.samples = static_cast<VkSampleCountFlagBits>(_sample_count);
    info.usage = _usage;
    info.mipLevels = _mip_levels;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (vkCreateImage(gpu->_logical_device, &info, nullptr, _vk_image.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create image");
    }
    // bind memory
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(gpu->_logical_device, *_vk_image, &mem_requirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = mem_requirements.size;
    allocInfo.memoryTypeIndex = _find_memory_type_index(mem_requirements.memoryTypeBits, _memory_type);
    if (vkAllocateMemory(gpu->_logical_device, &allocInfo, nullptr, &_memory) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate image memory!");
    }
    vkBindImageMemory(gpu->_logical_device, *_vk_image, _memory, 0);
}

void Image::allocate_vk_image_view()
{
    _vk_image_view.reset(new VkImageView, std::bind(&Image::_deallocate_image_view, gpu, std::placeholders::_1));
    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = *_vk_image;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = static_cast<VkFormat>(_format == Format::DEPTH ? gpu->depth_format().second : _format);
    info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = _mip_levels;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    if (vkCreateImageView(gpu->_logical_device, &info, nullptr, _vk_image_view.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create image view");
    }
}


uint32_t Image::_find_memory_type_index(uint32_t memory_type_bits, VkMemoryPropertyFlagBits _memory_type) const
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gpu->_physical_device, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memory_type_bits & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & _memory_type) == _memory_type) {
            return i;
        }
    }
    THROW_ERROR("failed to find suitable memory type!");
}


void Image::_deallocate_image(const std::shared_ptr<GPU>& gpu, VkImage* vk_image)
{
    vkDestroyImage(gpu->_logical_device, *vk_image, nullptr);
    delete vk_image;
}

void Image::_deallocate_image_view(const std::shared_ptr<GPU>& gpu, VkImageView* vk_image_view)
{
    vkDestroyImageView(gpu->_logical_device, *vk_image_view, nullptr);
    delete vk_image_view;
}

// void Image::upload_data(unsigned char* data)
// {
//     VkCommandBuffer commandBuffer = beginSingleTimeCommands();

//     VkBufferImageCopy region{};
//     region.bufferOffset = 0;
//     region.bufferRowLength = 0;
//     region.bufferImageHeight = 0;
//     region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//     region.imageSubresource.mipLevel = 0;
//     region.imageSubresource.baseArrayLayer = 0;
//     region.imageSubresource.layerCount = 1;
//     region.imageOffset = {0, 0, 0};
//     region.imageExtent = {
//         width,
//         height,
//         1
//     };

//     vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

//     endSingleTimeCommands(commandBuffer);
// }