#include <RenderEngine/graphics/Image.hpp>
using namespace RenderEngine;

Image::Image(const GPU& _gpu, const std::string& image_path, std::optional<Image::Format> _format) : gpu(&_gpu)
{
    width = 0;
    height = 0;
    if (_format.has_value())
    {
        format = _format.value();
    }
    else
    {
        format = Image::Format::RGBA;
    }
    allocate_vk_image();
}

Image::Image(const GPU& _gpu, uint32_t _width, uint32_t _height, Image::Format _format) : gpu(&_gpu), width(_width), height(_height), format(_format)
{
    allocate_vk_image();
}

Image::Image(const GPU& _gpu, uint32_t _width, uint32_t _height, Image::Format _format, const VkImage& vk_image) : gpu(&_gpu), width(_width), height(_height), format(_format)
{
    allocate_vk_image(std::optional<VkImage>(vk_image));
    allocate_vk_image_view();
}

Image::Image(const GPU& _gpu, uint32_t _width, uint32_t _height, Image::Format _format, const std::vector<unsigned char>& data) : gpu(&_gpu), width(_width), height(_height), format(_format)
{
    allocate_vk_image();
    allocate_vk_image_view();
}

Image::Image(const Image& other)
{
    operator=(other);
}

Image::~Image()
{
}

void Image::operator=(const Image& other)
{
    gpu = other.gpu;
    width = other.width;
    height = other.height;
    format = other.format;
    _vk_image = other._vk_image;
    _vk_image_view = other._vk_image_view;
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
        if (vkCreateImage(*gpu->_logical_device, &info, nullptr, _vk_image.get()) != VK_SUCCESS)
        {
            THROW_ERROR("failed to create image")
        }
    }
    _vk_image.reset(pointer, std::bind(_deallocate_vk_image, std::placeholders::_1, *gpu));
}

void Image::allocate_vk_image_view()
{
    _vk_image_view.reset(new VkImageView, std::bind(_deallocate_vk_image_view, std::placeholders::_1, *gpu));
    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = *_vk_image;
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
    if (vkCreateImageView(*gpu->_logical_device, &info, nullptr, _vk_image_view.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create image view")
    }
}

void Image::_deallocate_vk_image(VkImage* vk_image, const GPU& gpu)
{
    vkDestroyImage(*gpu._logical_device, *vk_image, nullptr);
    delete vk_image;
}

void Image::_deallocate_vk_image_view(VkImageView* vk_image_view, const GPU& gpu)
{
    vkDestroyImageView(*gpu._logical_device, *vk_image_view, nullptr);
    delete vk_image_view;
}