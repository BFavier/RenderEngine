#include <RenderEngine/graphics/Image.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/utilities/Functions.hpp>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stb/stb_image_resize.h>

using namespace RenderEngine;

Image::Image(const std::shared_ptr<GPU>& gpu, const std::string& file_path, std::optional<ImageFormat> requested_format,
             std::optional<uint32_t> resized_width, std::optional<uint32_t> resized_height) : _gpu(gpu)
{
    std::vector<uint8_t> pixels;
    std::tie(pixels, _width, _height, _format) = read_pixels_from_file(file_path, requested_format);
    if (resized_width.has_value() || resized_height.has_value())
    {
        if (!resized_width.has_value())
        {
            resized_width = (_width * resized_height.value()) / _height;
        }
        if (!resized_height.has_value())
        {
            resized_height = (_height * resized_width.value()) / _width;
        }
        if (_width != resized_width.value() || _height != resized_height.value())
        {
            pixels = Image::resize_pixels(pixels, _format, _width, _height, resized_width.value(), resized_height.value());
            _width = resized_width.value();
            _height = resized_height.value();
        }
    }
    _mip_levels = _mip_levels_count(_width, _height);
    _vk_image = _create_vk_image(gpu, _width, _height, _format, _mip_levels, AntiAliasing::X1);
    _vk_device_memory = std::get<0>(_allocate_vk_device_memory(gpu, _vk_image, 1));
    _bind_image_to_memory(gpu, _vk_image, _vk_device_memory, 0);
    _create_vk_image_view();
    _create_vk_sampler();
    upload_data(pixels);
}

Image::Image(const std::shared_ptr<GPU>& gpu, ImageFormat format, uint32_t width, uint32_t height, bool mipmaped, AntiAliasing sample_count) : _gpu(gpu)
{
    _width = width;
    _height = height;
    _format = format;
    _mip_levels = (mipmaped ? _mip_levels_count(_width, _height) : 1);
    _vk_image = _create_vk_image(gpu, width, height, format, _mip_levels, sample_count);
    _vk_device_memory = std::get<0>(_allocate_vk_device_memory(gpu, _vk_image, 1));
    _bind_image_to_memory(gpu, _vk_image, _vk_device_memory, 0);
    _create_vk_image_view();
    _create_vk_sampler();
}

Image::Image(const std::shared_ptr<GPU>& gpu, const VkImage& vk_image, const std::shared_ptr<VkDeviceMemory>& vk_device_memory,
             ImageFormat format, uint32_t width, uint32_t height, bool mipmaped) : _gpu(gpu)
{
    _vk_image = vk_image;
    _vk_device_memory = vk_device_memory;
    _width = width;
    _height = height;
    _format = format;
    _mip_levels = (mipmaped ? _mip_levels_count(_width, _height) : 1);
    _vk_image = vk_image;
    _create_vk_image_view();
    _create_vk_sampler();
}

Image::~Image()
{
    vkDestroySampler(_gpu->_logical_device, _vk_sampler, nullptr);
    vkDestroyImageView(_gpu->_logical_device, _vk_image_view, nullptr);
    // If the image is owned (not created by the swapchain), destroy it
    if (_vk_device_memory.get() != nullptr)
    {
        vkDestroyImage(_gpu->_logical_device, _vk_image, nullptr);
    }
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

uint32_t Image::mip_levels_count() const
{
    return _mip_levels;
}

uint32_t Image::channel_count() const
{
    return Image::format_channel_count(_format);
}

size_t Image::channel_bytes_size() const
{
    return Image::format_channel_bytessize(_format);
}

VkImage Image::_create_vk_image(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, ImageFormat format, uint32_t mip_levels, AntiAliasing sample_count)
{
    VkImage vk_image = VK_NULL_HANDLE;
    VkImageUsageFlags usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (format == ImageFormat::DEPTH)
    {
        usage = usage | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    else
    {
        usage = usage | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;// | VK_IMAGE_USAGE_STORAGE_BIT
    }
    VkImageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.imageType = VK_IMAGE_TYPE_2D;
    info.format = static_cast<VkFormat>(format == ImageFormat::DEPTH ? gpu->depth_format().second : format);
    info.extent.width = width;
    info.extent.height = height;
    info.extent.depth = 1; // 2D images only
    info.arrayLayers = 1;  // only one layer per image
    info.tiling = VK_IMAGE_TILING_OPTIMAL;
    info.samples = static_cast<VkSampleCountFlagBits>(sample_count);
    info.usage = usage;
    info.mipLevels = mip_levels;
    info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;
    info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if (vkCreateImage(gpu->_logical_device, &info, nullptr, &vk_image) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create image");
    }
    return vk_image;
}

std::tuple<std::shared_ptr<VkDeviceMemory>, std::size_t> Image::_allocate_vk_device_memory(const std::shared_ptr<GPU>& gpu, const VkImage& vk_image, uint32_t n_images)
{
    // query required memory properties
    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(gpu->_logical_device, vk_image, &mem_requirements);
    std::size_t required_memory_bytes = ((mem_requirements.size / mem_requirements.alignment) + (mem_requirements.size % mem_requirements.alignment == 0 ? 0 : 1)) * mem_requirements.alignment;
    // find suitable memory type/heap
    uint32_t memoryTypeIndex = std::numeric_limits<uint32_t>::max();
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(gpu->_physical_device, &memProperties);
    VkMemoryPropertyFlagBits memory_requirements = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((mem_requirements.memoryTypeBits & (1 << i))
            &&
            ((memProperties.memoryTypes[i].propertyFlags & memory_requirements) == memory_requirements))
        {
            memoryTypeIndex = i;
            break;
        }
    }
    if (memoryTypeIndex == std::numeric_limits<uint32_t>::max())
    {
        THROW_ERROR("failed to find suitable memory type!");
    }
    // allocate device memory
    std::shared_ptr<VkDeviceMemory> vk_device_memory(
        new VkDeviceMemory(),
        [gpu](VkDeviceMemory* vk_device_memory){vkFreeMemory(gpu->_logical_device, *vk_device_memory, nullptr);delete vk_device_memory;}
    );
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = required_memory_bytes * n_images;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    if (vkAllocateMemory(gpu->_logical_device, &allocInfo, nullptr, vk_device_memory.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate image memory!");
    }
    return std::make_pair(vk_device_memory, required_memory_bytes);
}

void Image::_bind_image_to_memory(const std::shared_ptr<GPU>& gpu, const VkImage& vk_image, const std::shared_ptr<VkDeviceMemory>& vk_device_memory, std::size_t offset)
{
    vkBindImageMemory(gpu->_logical_device, vk_image, *vk_device_memory.get(), offset);
}

uint32_t Image::_mip_levels_count(uint32_t width, uint32_t height)
{
    return std::min(std::log2(width), std::log2(height));
}

void Image::_create_vk_sampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = _gpu->_device_features.samplerAnisotropy;
    samplerInfo.maxAnisotropy = _gpu->_device_properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;
    if (vkCreateSampler(_gpu->_logical_device, &samplerInfo, nullptr, &_vk_sampler) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create texture sampler!");
    }
}

void Image::_create_vk_image_view()
{
    VkImageViewCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    info.image = _vk_image;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = static_cast<VkFormat>(_format == ImageFormat::DEPTH ? _gpu->depth_format().second : _format);
    info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    info.subresourceRange.aspectMask = _get_aspect_mask();
    info.subresourceRange.baseMipLevel = 0;
    info.subresourceRange.levelCount = _mip_levels;
    info.subresourceRange.baseArrayLayer = 0;
    info.subresourceRange.layerCount = 1;  // only one layer per image
    if (vkCreateImageView(_gpu->_logical_device, &info, nullptr, &_vk_image_view) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create image view");
    }
}

uint8_t Image::format_channel_count(const ImageFormat& format)
{
    if (format == ImageFormat::GRAY) {return 1;}
    else if (format == ImageFormat::RGBA) {return 4;}
    else if (format == ImageFormat::NORMAL) { return 4; }
    else if (format == ImageFormat::MATERIAL) { return 4; }
    else if (format == ImageFormat::FLOAT3) {return 3;}
    else if (format == ImageFormat::FLOAT4) {return 4;}
    else if (format == ImageFormat::DEPTH) {return 1;}
    else {THROW_ERROR("Unknown format '" + std::to_string(format) + "'.");}
}

size_t Image::format_channel_bytessize(const ImageFormat& format)
{
    if (format == ImageFormat::GRAY) {return 1;}
    else if (format == ImageFormat::RGBA) {return 1;}
    else if (format == ImageFormat::NORMAL) { return 1; }
    else if (format == ImageFormat::MATERIAL) { return 1; }
    else if (format == ImageFormat::FLOAT3) {return 4;}
    else if (format == ImageFormat::FLOAT4) {return 4;}
    else if (format == ImageFormat::DEPTH) {return 4;}
    else {THROW_ERROR("Unknown format '" + std::to_string(format) + "'.");}
}

std::vector<std::shared_ptr<Image>> Image::bulk_allocate_images(const std::shared_ptr<GPU>& gpu, uint32_t n_images, ImageFormat format, uint32_t width, uint32_t height, bool mipmaped)
{
    std::vector<std::shared_ptr<Image>> images;
    if (n_images == 0)
    {
        return images;
    }
    std::vector<VkImage> vk_images;
    for (uint32_t i=0; i<n_images; i++)
    {
        vk_images.push_back(_create_vk_image(gpu, width, height, format, _mip_levels_count(width, height), AntiAliasing::X1));
    }
    std::size_t offset;
    std::shared_ptr<VkDeviceMemory> vk_device_memory;
    std::tie(vk_device_memory, offset) = _allocate_vk_device_memory(gpu, vk_images[0], vk_images.size());
    for (uint32_t i=0; i<n_images; i++)
    {
        _bind_image_to_memory(gpu, vk_images[i], vk_device_memory, i*offset);
    }
    for (uint32_t i=0; i<n_images; i++)
    {
        Image* image = new Image(gpu, vk_images[i], vk_device_memory, format, width, height, mipmaped);
        images.emplace_back(image);
    }
    return images;
}

std::vector<std::shared_ptr<Image>> Image::bulk_load_images(const std::shared_ptr<GPU>& gpu, const std::vector<std::string>& file_paths, ImageFormat format, uint32_t width, uint32_t height, bool mipmaped)
{
    std::vector<std::shared_ptr<Image>> images = bulk_allocate_images(gpu, file_paths.size(), format, width, height, mipmaped);
    for (std::size_t i=0; i<file_paths.size(); i++)
    {
        std::vector<uint8_t> pixels;
        uint32_t r_width, r_height;
        ImageFormat _f;
        std::tie(pixels, r_width, r_height, _f) = Image::read_pixels_from_file(file_paths[i], format);
        if (r_width != width || r_height != height)
        {
            pixels = Image::resize_pixels(pixels, format, r_width, r_height, width, height);
        }
        images[i]->upload_data(pixels);
    }
    return images;
}

std::map<std::string, std::shared_ptr<Image>> Image::bulk_load_images(const std::shared_ptr<GPU>& gpu, const std::map<std::string, std::string>& resource_paths, ImageFormat format, uint32_t width, uint32_t height, bool mipmaped)
{
    std::vector<std::string> keys;
    std::vector<std::string> file_paths;
    for (const std::pair<std::string, std::string>& resource_path : resource_paths)
    {
        keys.push_back(resource_path.first);
        file_paths.push_back(resource_path.second);
    }
    std::vector<std::shared_ptr<Image>> images = Image::bulk_load_images(gpu, file_paths, format, width, height, mipmaped);
    std::map<std::string, std::shared_ptr<Image>> resources;
    for (std::size_t i=0; i<resource_paths.size(); i++)
    {
        resources[keys[i]] = images[i];
    }
    return resources;
}

std::tuple<std::vector<uint8_t>, uint32_t, uint32_t, ImageFormat> Image::read_pixels_from_file(const std::string& file_path, const std::optional<ImageFormat>& read_format)
{
    //TODO : improve the image format handling
    ImageFormat format;
    int i_width, i_height, n_channels, n_required_channels;
    if (read_format.has_value())
    {
        format = read_format.value();
        if (format != ImageFormat::GRAY && format != ImageFormat::RGBA)
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
        else if (n_channels == 3) {THROW_ERROR("Reading RGB image is not implemented, specify read_format=RGBA");format = ImageFormat::RGBA;}
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

std::vector<uint8_t> Image::resize_pixels(const std::vector<uint8_t>& pixels, ImageFormat format, uint32_t width, uint32_t height, uint32_t new_width, uint32_t new_height)
{
    int result;
    std::vector<uint8_t> resized_pixels(new_width*new_height*format_channel_count(format)*format_channel_bytessize(format));
    if (format == ImageFormat::GRAY || format == ImageFormat::RGBA)
    {
        result = stbir_resize_uint8(pixels.data(), width, height, 0,
                                    resized_pixels.data(), new_width, new_height, 0,
                                    format_channel_count(format));
    }
    else if (format == ImageFormat::FLOAT3 || format == ImageFormat::FLOAT4)
    {
        result = stbir_resize_float(reinterpret_cast<const float*>(pixels.data()), width, height, 0,
                                    reinterpret_cast<float*>(resized_pixels.data()), new_width, new_height, 0,
                                    format_channel_count(format));
    }
    else
    {
        THROW_ERROR("Unsupported ImageFormat "+std::to_string(format));
    }
    return resized_pixels;
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

void Image::_transition_to_layout(VkImageLayout new_layout, VkCommandBuffer command_buffer)
{
    if (new_layout == _current_layout)
    {
        return;
    }
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = _current_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = _vk_image;
    barrier.subresourceRange.aspectMask = _get_aspect_mask();
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = _mip_levels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;  // will be modified later in the function
    barrier.dstAccessMask = 0;  // will be modified later in the function
    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;
    _fill_layout_attributes(_current_layout, barrier.srcQueueFamilyIndex, barrier.srcAccessMask, source_stage);
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
        VkFormat depth_format = _gpu->depth_format().second;
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
        _current_queue = _gpu->_graphics_queue.value();
    }
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = std::get<2>(_current_queue.value());
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(_gpu->_logical_device, &allocInfo, &commandBuffer);
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
        _current_queue = _gpu->_graphics_queue.value();
    }
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(std::get<1>(_current_queue.value()), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(std::get<1>(_current_queue.value()));
    vkFreeCommandBuffers(_gpu->_logical_device, std::get<2>(_current_queue.value()), 1, &commandBuffer);
}

void Image::save_to_disk(const std::string& file_path)
{
    std::vector<uint8_t> pixels = download_data();
    Image::save_pixels_to_file(file_path, pixels, _width, _height, _format);
}

void Image::upload_data(const std::vector<uint8_t>& pixels)
{
    size_t image_size = width()*(height()*(channel_count()*channel_bytes_size()));
    if (pixels.size() != image_size)
    {
        THROW_ERROR("pixel vector has not the right size.")
    }
    uint8_t n_channels = Image::format_channel_count(_format);
    Buffer staging_buffer(_gpu, image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
    staging_buffer.upload(reinterpret_cast<const uint8_t*>(pixels.data()), staging_buffer.bytes_size(), 0);
    VkCommandBuffer commandBuffer = _begin_single_time_commands();
    _transition_to_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, commandBuffer);
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;// | VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = pixels.size() / image_size;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width(), height(), 1};
    vkCmdCopyBufferToImage(commandBuffer, staging_buffer._vk_buffer, _vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    _end_single_time_commands(commandBuffer);
}

std::vector<uint8_t> Image::download_data()
{
    uint8_t n_channels = Image::format_channel_count(_format);
    Buffer staging_buffer(_gpu, width()*height()*channel_count()*channel_bytes_size(), VK_BUFFER_USAGE_TRANSFER_DST_BIT);
    std::vector<uint8_t> pixels(staging_buffer.bytes_size());
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
    vkCmdCopyImageToBuffer(commandBuffer, _vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging_buffer._vk_buffer, 1, &region);
    _end_single_time_commands(commandBuffer);
    staging_buffer.download(pixels.data(), staging_buffer.bytes_size(), 0);
    return pixels;
}
