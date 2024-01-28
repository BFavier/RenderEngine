#include <RenderEngine/graphics/Image.hpp>
using namespace RenderEngine;

Image::Image(const GPU& _gpu, const std::string& image_path, std::optional<Image::Format> _format) : gpu(_gpu)
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
        format = _format.value();
    }
    else
    {
        format = Image::Format::RGBA;
    }
    allocate_vk_image();
    stbi_image_free(imgData);
}

Image::Image(const GPU& _gpu, uint32_t _width, uint32_t _height, Image::Format _format) : gpu(_gpu), width(_width), height(_height), format(_format)
{
    allocate_vk_image();
    allocate_vk_image_view();
}

Image::Image(const GPU& _gpu, uint32_t _width, uint32_t _height, Image::Format _format, const VkImage& vk_image) : gpu(_gpu), width(_width), height(_height), format(_format)
{
    allocate_vk_image(std::optional<VkImage>(vk_image));
    allocate_vk_image_view();
}

Image::Image(const GPU& _gpu, uint32_t _width, uint32_t _height, Image::Format _format, const std::vector<unsigned char>& data) : gpu(_gpu), width(_width), height(_height), format(_format)
{
    allocate_vk_image();
    allocate_vk_image_view();
}

Image::~Image()
{
    vkDestroyImageView(gpu._logical_device, _vk_image_view, nullptr);
    vkDestroyImage(gpu._logical_device, _vk_image, nullptr);
}

void Image::allocate_vk_image(const std::optional<VkImage>& image)
{
    VkImage* pointer = new VkImage();
    if (image.has_value())
    {
        *pointer = image.value();
    }
    else
    {
        VkImageCreateInfo info{};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = static_cast<VkFormat>(format);
        info.extent.width = width;
        info.extent.height = height;
        info.extent.depth = 1;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        if (vkCreateImage(gpu._logical_device, &info, nullptr, pointer) != VK_SUCCESS)
        {
            THROW_ERROR("failed to create image")
        }
    }
}

void Image::allocate_vk_image_view()
{
    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = _vk_image;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = static_cast<VkFormat>(format);
    info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;
    if (vkCreateImageView(gpu._logical_device, &info, nullptr, &_vk_image_view) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create image view")
    }
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