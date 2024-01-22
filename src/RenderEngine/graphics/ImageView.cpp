#include <RenderEngine/graphics/ImageView.hpp>
#include <RenderEngine/graphics/Image.hpp>
using namespace RenderEngine;

ImageView::ImageView(const Image& _image, unsigned int x, unsigned int y, unsigned int width, unsigned int height) : image(_image)
{
    _set_vk_image_view();
    _set_graphic_pipeline();
}

ImageView::~ImageView()
{
    vkDestroyImageView(image.gpu._logical_device, _vk_image_view, nullptr);
    vkDestroyPipelineLayout(image.gpu._logical_device, _vk_graphic_pipeline, nullptr);
}

void ImageView::_set_vk_image_view()
{
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image._vk_image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = image._vk_image_format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(image.gpu._logical_device, &createInfo, nullptr, &_vk_image_view) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create image view")
    }
}

void ImageView::_set_graphic_pipeline()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
    if (vkCreatePipelineLayout(image.gpu._logical_device, &pipelineLayoutInfo, nullptr, &_vk_graphic_pipeline) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create pipeline layout!")
    }
}