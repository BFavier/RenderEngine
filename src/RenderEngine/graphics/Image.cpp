#include <RenderEngine/graphics/Image.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/utilities/Functions.hpp>
using namespace RenderEngine;

Image::Image(const std::shared_ptr<GPU>& _gpu, const std::string& file_path, std::optional<ImageFormat> requested_format,
             bool texture_compatible, bool storage_compatible, VkMemoryPropertyFlags memory_type, AntiAliasing sample_count) : gpu(_gpu)
{
    std::vector<uint8_t> pixels;
    ImageFormat format;
    uint32_t width, height;
    std::tie<std::vector<uint8_t>, uint32_t, uint32_t, ImageFormat>(pixels, width, height, format) = read_pixels_from_file(file_path, requested_format);
    _allocate_vk_image(width, height, 1, format, texture_compatible, storage_compatible, memory_type, sample_count);
    _allocate_vk_image_view();
    upload_data(pixels);
}

Image::Image(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height, uint32_t layers, ImageFormat format, bool texture_compatible, bool storage_compatible, VkMemoryPropertyFlags memory_type, AntiAliasing sample_count) : gpu(_gpu)
{
    _allocate_vk_image(width, height, layers, format, texture_compatible, storage_compatible, memory_type, sample_count);
    _allocate_vk_image_view();
}

Image::Image(const std::shared_ptr<GPU>& _gpu, const std::shared_ptr<VkImage>& vk_image, uint32_t width, uint32_t height, ImageFormat format, bool texture_compatible, bool storage_compatible, AntiAliasing sample_count) : gpu(_gpu)
{
    _set_attributes(width, height, 1, format, texture_compatible, storage_compatible, sample_count);
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

ImageFormat Image::format() const
{
    return _format;
}

uint32_t Image::layers() const
{
    return _layers;
}

uint32_t Image::channel_count() const
{
    return Image::format_channel_count(_format);
}

size_t Image::channel_bytes_size() const
{
    return Image::format_channel_bytessize(_format);
}

bool Image::is_texture_compatible() const
{
    return _texture_compatible;
}

void Image::_set_attributes(uint32_t width, uint32_t height, uint32_t layers, ImageFormat format, bool texture_compatible, bool storage_compatible, AntiAliasing sample_count)
{
    _width = width;
    _height = height;
    _layers = layers;
    _format = format;
    _texture_compatible = texture_compatible;
    if (texture_compatible)
    {
        _mip_levels = std::min(static_cast<uint32_t>(std::log2(width)), static_cast<uint32_t>(std::log2(height)));
    }
    else
    {
        _mip_levels = 1;
    }
    _storage_compatible = storage_compatible;
    _sample_count = sample_count;
    _layout = VK_IMAGE_LAYOUT_UNDEFINED;
    _current_queue = std::nullopt;
}

void Image::_allocate_vk_image(uint32_t width, uint32_t height, uint32_t layers, ImageFormat format, bool texture_compatible, bool storage_compatible, VkMemoryPropertyFlags memory_type, AntiAliasing sample_count)
{
    _set_attributes(width, height, layers, format, texture_compatible, storage_compatible, sample_count);
    _vk_image.reset(new VkImage, std::bind(&Image::_deallocate_image, gpu, std::placeholders::_1));
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (_format == ImageFormat::DEPTH)
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
    if (storage_compatible)
    {
        usage = usage | VK_IMAGE_USAGE_STORAGE_BIT;
    }
    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = static_cast<VkFormat>(_format == ImageFormat::DEPTH ? gpu->depth_format().second : _format);
    info.extent.width = _width;
    info.extent.height = _height;
    info.extent.depth = 1; // 2D images only
    info.arrayLayers = _layers;
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
    std::shared_ptr<GPU>& _gpu = gpu;
    _vk_device_memory.reset(new VkDeviceMemory, [_gpu](VkDeviceMemory* mem) {Image::_free_image_memory(_gpu, mem);});
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(gpu->_logical_device, *_vk_image, &mem_requirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = mem_requirements.size;
    allocInfo.memoryTypeIndex = _find_memory_type_index(mem_requirements.memoryTypeBits, memory_type);
    if (vkAllocateMemory(gpu->_logical_device, &allocInfo, nullptr, _vk_device_memory.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate image memory!");
    }
    vkBindImageMemory(gpu->_logical_device, *_vk_image, *_vk_device_memory.get(), 0);
    // create sampler
    _vk_sampler.reset(new VkSampler, [_gpu](VkSampler* sampler){_deallocate_sampler(_gpu, sampler);});
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = gpu->_device_features.samplerAnisotropy;
    samplerInfo.maxAnisotropy = gpu->_device_properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    if (vkCreateSampler(gpu->_logical_device, &samplerInfo, nullptr, _vk_sampler.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create texture sampler!");
    }
}

void Image::_allocate_vk_image_view()
{
    _vk_image_view.reset(new VkImageView, std::bind(&Image::_deallocate_image_view, gpu, std::placeholders::_1));
    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = *_vk_image;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = static_cast<VkFormat>(_format == ImageFormat::DEPTH ? gpu->depth_format().second : _format);
    info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.subresourceRange.aspectMask = _get_aspect_mask();
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = _mip_levels;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = _layers;
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

uint8_t Image::format_channel_count(const ImageFormat& format)
{
    if (format == ImageFormat::GRAY) {return 1;}
    else if(format == ImageFormat::RGB) {return 3;}
    else if (format == ImageFormat::RGBA) {return 4;}
    else if (format == ImageFormat::POINTER) {return 1;}
    else if (format == ImageFormat::FLOAT3) {return 3;}
    else if (format == ImageFormat::FLOAT4) {return 4;}
    else if (format == ImageFormat::DEPTH) {return 1;}
    else {THROW_ERROR("Unknown format '" + std::to_string(format) + "'.");}
}

size_t Image::format_channel_bytessize(const ImageFormat& format)
{
    if (format == ImageFormat::GRAY) {return 1;}
    else if(format == ImageFormat::RGB) {return 1;}
    else if (format == ImageFormat::RGBA) {return 1;}
    else if (format == ImageFormat::POINTER) {return sizeof(void*);}
    else if (format == ImageFormat::FLOAT3) {return 4;}
    else if (format == ImageFormat::FLOAT4) {return 4;}
    else if (format == ImageFormat::DEPTH) {return 4;}
    else {THROW_ERROR("Unknown format '" + std::to_string(format) + "'.");}
}

std::tuple<std::vector<uint8_t>, uint32_t, uint32_t, ImageFormat> Image::read_pixels_from_file(const std::string& file_path, const std::optional<ImageFormat>& read_format)
{
    ImageFormat format;
    int i_width, i_height, n_channels, n_required_channels;
    if (read_format.has_value())
    {
        format = read_format.value();
        if (format != ImageFormat::GRAY && format != ImageFormat::RGB && format != ImageFormat::RGBA)
        {
            THROW_ERROR("Reading pixels format '" + std::to_string(format) + "' from an image file is not supported.");
        }
        n_required_channels = Image::format_channel_count(format);
    }
    else
    {
        n_required_channels = 0;
    }
    uint8_t* img_data = stbi_load(file_path.c_str(), &i_width, &i_height, &n_channels, n_required_channels);
    if (!read_format.has_value())
    {
        std::string upper_file_path = Utilities::to_upper(file_path);
        if (Utilities::ends_with(upper_file_path, ".HDR") && n_channels == 3) {format = ImageFormat::FLOAT3;}
        else if (Utilities::ends_with(upper_file_path, ".HDR") && n_channels == 4) {format = ImageFormat::FLOAT4;}
        else if (n_channels == 4) {format = ImageFormat::RGBA;}
        else if (n_channels == 3) {format = ImageFormat::RGB;}
        else if (n_channels == 1) {format = ImageFormat::GRAY;}
        else {THROW_ERROR("Unexpected number of channels in image file");}
    }
    else
    {
        format = read_format.value();
    }
    uint32_t bytes_size = Image::format_channel_bytessize(format);
    // creating the vector of data
    std::vector<uint8_t> pixels;
    pixels.reserve(i_height * i_width * n_channels * bytes_size);
    for (uint32_t i=0;i<i_height;i++)
    {
        for (uint32_t j=0;j<i_width;j++)
        {
            for (uint32_t k=0;k<n_channels;k++)
            {
                for (uint32_t l=0;l<bytes_size;l++)
                {
                    pixels.push_back(img_data[pixels.size()]);
                }
            }
        }
    }
    stbi_image_free(img_data);
    return std::make_tuple(pixels, static_cast<uint32_t>(i_width), static_cast<uint32_t>(i_height), format);
}

void Image::save_pixels_to_file(const std::string& file_path, const std::vector<uint8_t>& pixels, uint32_t& width, uint32_t& height, ImageFormat format)
{
    std::string upper_file_path = Utilities::to_upper(file_path);
    int result;
    unsigned int channels = Image::format_channel_count(format);
    if (Utilities::ends_with(upper_file_path, ".PNG"))
    {
        result = stbi_write_png(file_path.c_str(), width, height, channels, pixels.data(), width * channels);
    }
    else if (Utilities::ends_with(upper_file_path, ".JPG") || Utilities::ends_with(upper_file_path, ".JPEG"))
    {
        result = stbi_write_jpg(file_path.c_str(), width, height, channels, pixels.data(), 100);
    }
    else if (Utilities::ends_with(upper_file_path, ".BMP"))
    {
        result = stbi_write_bmp(file_path.c_str(), width, height, channels, pixels.data());
    }
    else if (Utilities::ends_with(upper_file_path, ".TGA"))
    {
        result = stbi_write_tga(file_path.c_str(), width, height, channels, pixels.data());
    }
    else if (Utilities::ends_with(upper_file_path, ".HDR"))
    {
        result = stbi_write_hdr(file_path.c_str(), width, height, channels, reinterpret_cast<const float*>(pixels.data()));
    }
    else
    {
        THROW_ERROR("Unsupported image file extension");
    }
    if (result == 0)
    {
        THROW_ERROR("Failed to write image.");
    }
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

void Image::_free_image_memory(const std::shared_ptr<GPU>& gpu, VkDeviceMemory* vk_device_memory)
{
    vkFreeMemory(gpu->_logical_device, *vk_device_memory, nullptr);
    *vk_device_memory = VK_NULL_HANDLE;
}

void Image::_deallocate_sampler(const std::shared_ptr<GPU>& gpu, VkSampler* vk_sampler)
{
    vkDestroySampler(gpu->_logical_device, *vk_sampler, nullptr);;
    *vk_sampler = nullptr;
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
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
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
    _fill_layout_attributes(new_layout, barrier.dstQueueFamilyIndex, barrier.dstAccessMask, destination_stage);
    vkCmdPipelineBarrier(
        command_buffer,
        source_stage, destination_stage,
        VK_DEPENDENCY_BY_REGION_BIT,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void Image::_fill_layout_attributes(VkImageLayout layout, uint32_t& queue_family_index, VkAccessFlags& acces_mask, VkPipelineStageFlags& stage) const
{
    if (layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
    {
        acces_mask = VK_ACCESS_SHADER_WRITE_BIT;
        stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if(layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        acces_mask = VK_ACCESS_SHADER_READ_BIT;
        stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        acces_mask = VK_ACCESS_TRANSFER_READ_BIT;
        stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
    {
        acces_mask = VK_ACCESS_TRANSFER_WRITE_BIT;
        stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (layout == VK_IMAGE_LAYOUT_UNDEFINED)
    {
        acces_mask = 0;
        stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    else if (layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
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
    if (_format == ImageFormat::DEPTH)
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

VkCommandBuffer Image::_begin_single_time_commands()
{
    if (!_current_queue.has_value())
    {
        _current_queue = gpu->_graphics_queue.value();
    }
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = std::get<2>(_current_queue.value());
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(gpu->_logical_device, &allocInfo, &commandBuffer);
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void Image::_end_single_time_commands(VkCommandBuffer commandBuffer)
{
    if (!_current_queue.has_value())
    {
        _current_queue = gpu->_graphics_queue.value();
    }
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(std::get<1>(_current_queue.value()), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(std::get<1>(_current_queue.value()));
    vkFreeCommandBuffers(gpu->_logical_device, std::get<2>(_current_queue.value()), 1, &commandBuffer);
}

AntiAliasing Image::sample_count() const
{
    return _sample_count;
}

void Image::save_to_disk(const std::string& file_path)
{
    std::vector<uint8_t> pixels = download_data();
    Image::save_pixels_to_file(file_path, pixels, _width, _height, _format);
}

void Image::upload_data(const std::vector<uint8_t>& pixels, uint32_t layer_offset)
{
    size_t image_size = width()*(height()*(channel_count()*channel_bytes_size()));
    if ((pixels.size() % image_size != 0) || (pixels.size() < image_size))
    {
        THROW_ERROR("pixel vector has not the right size.")
    }
    uint8_t n_channels = Image::format_channel_count(_format);
    Buffer staging_buffer(gpu, n_channels*_width*_height,
                          VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    staging_buffer.upload(pixels.data());
    VkCommandBuffer commandBuffer = _begin_single_time_commands();
    _transition_to_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;// | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = layer_offset;
    region.imageSubresource.layerCount = pixels.size() / image_size;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width(), height(), 1};
    vkCmdCopyBufferToImage(commandBuffer, *staging_buffer._vk_buffer, *_vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    _end_single_time_commands(commandBuffer);
}

std::vector<uint8_t> Image::download_data(uint32_t layer)
{
    uint8_t n_channels = Image::format_channel_count(_format);
    Buffer staging_buffer(gpu, n_channels*_width*_height,
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    std::vector<uint8_t> pixels(width()*height()*channel_count()*channel_bytes_size());
    VkCommandBuffer commandBuffer = _begin_single_time_commands();
    _transition_to_layout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, commandBuffer);
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width(), height(), 1};
    vkCmdCopyImageToBuffer(commandBuffer, *_vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *staging_buffer._vk_buffer, 1, &region);
    _end_single_time_commands(commandBuffer);
    staging_buffer.download(pixels.data());
    return pixels;
}