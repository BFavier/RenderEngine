#include <RenderEngine/graphics/Image.hpp>
#define STB_IMAGE_IMPLEMENTATION  
#include <stb/stb_image.h>
using namespace RenderEngine;

Image::Image(const std::shared_ptr<GPU>& _gpu, const std::string& file_path, std::optional<Format> format,
             Image::AntiAliasing sample_count, bool texture_compatible, VkMemoryPropertyFlags memory_type) : gpu(_gpu)
{
    int i_width, i_height, n_channels, n_required_channels;
    if (format.has_value())
    {
        switch (format.value())
        {
            case Format::GRAY:
                n_required_channels = 1;
                break;
            case Format::UV:
                n_required_channels = 2;
                break;
            case Format::RGB:
                n_required_channels = 3;
                break;
            case Format::RGBA:
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
    unsigned char* imgData = stbi_load(file_path.c_str(), &i_width, &i_height, &n_channels, n_required_channels);
    if (!format.has_value())
    {
        if (n_channels == 4)
        {
            format = Format::RGBA;
        }
        else if (n_channels == 3)
        {
            format = Format::RGB;
        }
        else if (n_channels == 1)
        {
            format = Format::GRAY;
        }
        else
        {
            THROW_ERROR("Unexpected number of channels in image file")
        }
    }
    _allocate_vk_image(i_width, i_height, format.value(), sample_count, texture_compatible, memory_type);
    _allocate_vk_image_view();
    // upload_data(...);
    stbi_image_free(imgData);
}

Image::Image(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height, Format format, AntiAliasing sample_count, bool texture_compatible, VkMemoryPropertyFlags memory_type) : gpu(_gpu)
{
    _allocate_vk_image(width, height, format, sample_count, texture_compatible, memory_type);
    _allocate_vk_image_view();
}

Image::Image(const std::shared_ptr<GPU>& _gpu, const std::shared_ptr<VkImage>& vk_image, uint32_t width, uint32_t height, Format format, AntiAliasing sample_count, bool texture_compatible) : gpu(_gpu)
{
    _set_attributes(width, height, format, sample_count, texture_compatible);
    _vk_image = vk_image;
    _allocate_vk_image_view();
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

Format Image::format() const
{
    return _format;
}

Image::AntiAliasing Image::sample_count() const
{
    return _sample_count;
}

bool Image::is_texture_compatible() const
{
    return _texture_compatible;
}

void Image::_set_attributes(uint32_t width, uint32_t height, Format format, Image::AntiAliasing sample_count, bool texture_compatible)
{
    _width = width;
    _height = height;
    _format = format;
    _sample_count = sample_count;
    _texture_compatible = texture_compatible;
    if (texture_compatible)
    {
        _mip_levels = std::min(static_cast<uint32_t>(std::log2(width)), static_cast<uint32_t>(std::log2(height)));
    }
    else
    {
        _mip_levels = 1;
    }
    _layout = VK_IMAGE_LAYOUT_UNDEFINED;
}

void Image::_allocate_vk_image(uint32_t width, uint32_t height, Format format, Image::AntiAliasing sample_count, bool texture_compatible, VkMemoryPropertyFlags memory_type)
{
    _set_attributes(width, height, format, sample_count, texture_compatible);
    _vk_image.reset(new VkImage, std::bind(&Image::_deallocate_image, gpu, std::placeholders::_1));
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (_format == Format::DEPTH)
    {
        usage = usage | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    else
    {
        usage = usage | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    if (texture_compatible)
    {
        usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = static_cast<VkFormat>(_format == Format::DEPTH ? gpu->depth_format().second : _format);
    info.extent.width = _width;
    info.extent.height = _height;
    info.extent.depth = 1; // 2D images only
    info.arrayLayers = 1; // Only one image is allocated
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.samples = static_cast<VkSampleCountFlagBits>(sample_count);
    info.usage = usage;
    info.mipLevels = _mip_levels;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;
    info.initialLayout = _layout;
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
    allocInfo.memoryTypeIndex = _find_memory_type_index(mem_requirements.memoryTypeBits, memory_type);
    if (vkAllocateMemory(gpu->_logical_device, &allocInfo, nullptr, &_memory) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate image memory!");
    }
    vkBindImageMemory(gpu->_logical_device, *_vk_image, _memory, 0);
}

void Image::_allocate_vk_image_view()
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
    info.subresourceRange.aspectMask = _get_aspect_mask();
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = _mip_levels;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    if (vkCreateImageView(gpu->_logical_device, &info, nullptr, _vk_image_view.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create image view");
    }
}


uint32_t Image::_find_memory_type_index(uint32_t memory_type_bits, VkMemoryPropertyFlags _memory_type) const
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

void Image::_transition_to_layout(VkImageLayout new_layout, VkCommandBuffer command_buffer)
{
    if (new_layout == _layout)
    {
        return;
    }
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = _layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // will be modified later in the function
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;  // will be modified later in the function
    barrier.image = *_vk_image;
    barrier.subresourceRange.aspectMask = _get_aspect_mask();
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = _mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;  // will be modified later in the function
    barrier.dstAccessMask = 0;  // will be modified later in the function
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;
    _fill_layout_attributes(_layout, barrier.srcQueueFamilyIndex, barrier.srcAccessMask, source_stage);
    _fill_layout_attributes(_layout, barrier.dstQueueFamilyIndex, barrier.dstAccessMask, destination_stage);
    if (barrier.srcQueueFamilyIndex == barrier.dstQueueFamilyIndex)
    {
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }
    vkCmdPipelineBarrier(
        command_buffer,
        source_stage, destination_stage,
        VK_DEPENDENCY_BY_REGION_BIT,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void Image::_fill_layout_attributes(VkImageLayout layout, uint32_t& queue_family_index, VkAccessFlags& acces_mask, VkPipelineStageFlags& stage)
{
    if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        queue_family_index = std::get<0>(gpu->_graphics_queue.value());
        acces_mask = VK_ACCESS_SHADER_WRITE_BIT;
        stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if(layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        queue_family_index = std::get<0>(gpu->_graphics_queue.value());
        acces_mask = VK_ACCESS_SHADER_READ_BIT;
        stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        queue_family_index = std::get<0>(gpu->_transfer_queue.value());
        acces_mask = VK_ACCESS_TRANSFER_READ_BIT;
        stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        queue_family_index = std::get<0>(gpu->_transfer_queue.value());
        acces_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
        stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (layout == VK_IMAGE_LAYOUT_UNDEFINED)
    {
        queue_family_index = 0;
        acces_mask = 0;
        stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else if (layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        queue_family_index = std::get<0>(gpu->_graphics_queue.value());
        acces_mask = VK_ACCESS_SHADER_WRITE_BIT;
        stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        THROW_ERROR("Unexpected layout")
    }
}


VkImageAspectFlags Image::_get_aspect_mask() const
{
    VkImageAspectFlags aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    if (_format == Format::DEPTH)
    {
        VkFormat depth_format = gpu->depth_format().second;
        if (depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT)
        {
            aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        else
        {
            aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
    }
    return aspect_mask;
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