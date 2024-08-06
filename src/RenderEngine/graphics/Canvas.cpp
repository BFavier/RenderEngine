#include <RenderEngine/render_engine.hpp>
#include <RenderEngine/graphics/Canvas.hpp>
#include <RenderEngine/scene/model/Mesh.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>
#include <RenderEngine/utilities/Macro.hpp>
using namespace RenderEngine;


Canvas::Canvas(const GPU* _gpu, uint32_t _width, uint32_t _height, bool mip_maped, AntiAliasing sample_count) :
    gpu(_gpu),
    images({{"color", new Image(_gpu, ImageFormat::RGBA, _width, _height, mip_maped)},
            {"albedo", new Image(_gpu, ImageFormat::RGBA, _width, _height, mip_maped)},
            {"normal", new Image(_gpu, ImageFormat::NORMAL, _width, _height, mip_maped)},
            {"material", new Image(_gpu, ImageFormat::MATERIAL, _width, _height, mip_maped)},
            {"depth", new Image(_gpu, ImageFormat::DEPTH, _width, _height, false)}}),
    width(_width), height(_height), _final_layout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
{
    for (std::pair<std::string, Shader*> shader : gpu->_shaders)
    {
        _frame_buffers[shader.second] = _allocate_frame_buffer(shader.second);
    }
    _allocate_command_buffer(_vk_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    _allocate_fence(_vk_fence);
    _allocate_semaphore(_vk_rendered_semaphore);
}


Canvas::Canvas(const GPU* _gpu, const VkImage& vk_image, uint32_t _width, uint32_t _height, AntiAliasing sample_count) :
    gpu(_gpu),
    images({{"color", new Image(_gpu, vk_image, nullptr, ImageFormat::RGBA, _width, _height, false)},
            {"albedo", new Image(_gpu, ImageFormat::RGBA, _width, _height, false)},
            {"normal", new Image(_gpu, ImageFormat::NORMAL, _width, _height, false)},
            {"material", new Image(_gpu, ImageFormat::MATERIAL, _width, _height, false)},
            {"depth", new Image(_gpu, ImageFormat::DEPTH, _width, _height, false)}}),
    width(_width), height(_height), _final_layout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
{
    for (std::pair<std::string, Shader*> shader : gpu->_shaders)
    {
        _frame_buffers[shader.second] = _allocate_frame_buffer(shader.second);
    }
    _allocate_command_buffer(_vk_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    _allocate_fence(_vk_fence);
    _allocate_semaphore(_vk_rendered_semaphore);
}


Canvas::~Canvas()
{
    vkDeviceWaitIdle(gpu->_logical_device);
    vkDestroySemaphore(gpu->_logical_device, _vk_rendered_semaphore, nullptr);
    vkDestroyFence(gpu->_logical_device, _vk_fence, nullptr);
    vkFreeCommandBuffers(gpu->_logical_device, std::get<2>(gpu->_graphics_queue.value()), 1, &_vk_command_buffer);
    for (const std::pair<const std::string, Image*>& image : images)
    {
        delete image.second;
    }
    for (std::pair<const Shader*, VkFramebuffer> frame_buffer : _frame_buffers)
    {
        vkDestroyFramebuffer(gpu->_logical_device, frame_buffer.second, nullptr);
    }
}



void Canvas::clear()
{
    _record_commands();
    Shader* shader = gpu->_shaders.at("Clear");
    _bind_shader(shader, images);
    vkCmdSetCullMode(_vk_command_buffer, VK_CULL_MODE_NONE);
    vkCmdDraw(_vk_command_buffer, 6, 1, 0, 0);
    for (std::pair<std::string, VkImageLayout> layout : shader->_final_layouts)
    {
        images.at(layout.first)->_current_layout = layout.second;
    }
}



/*
// TODO : replace transfer operations with a shader
void Canvas::clear(Color _color)
{
    _record_commands();
    // transition layout
    std::map<std::string, VkImageLayout> transitions;
    for (const std::pair<const std::string, Image*>& image : images)
    {
        transitions[image.first] = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }
    _command_barrier(transitions, images);
    // clear images
    for (const std::pair<const std::string, Image*>& image : images)
    {
        if (image.first == "depth")
        {
            VkClearDepthStencilValue clear_depth = {1.f, 0};
            VkImageSubresourceRange depth_range = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, image.second->mip_levels_count(), 0, 1};
            vkCmdClearDepthStencilImage(_vk_command_buffer, image.second->_vk_image, image.second->_current_layout, &clear_depth, 1, &depth_range);
        }
        else
        {
            VkClearColorValue clear_color_albedo = {{_color.r, _color.g, _color.b, _color.a}};
            VkClearColorValue clear_zero_albedo = {{0.0f, 0.0f, 0.0f, 0.0f}};
            VkImageSubresourceRange albedo_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, image.second->mip_levels_count(), 0, 1};
            vkCmdClearColorImage(_vk_command_buffer, image.second->_vk_image, image.second->_current_layout, ((image.first == "color") ? &clear_color_albedo : &clear_zero_albedo), 1, &albedo_range);
        }
    }
}
*/


void Canvas::draw(const Camera& camera, const std::shared_ptr<Mesh>& mesh, const std::tuple<Vector, Quaternion, double>& mesh_coordinates_in_camera, bool cull_back_faces)
{
    _record_commands();
    Shader* shader = gpu->_shaders.at("3D");
    _bind_shader(shader, images);
    // set culling mode
    if (gpu->dynamic_culling_supported())
    {
        vkCmdSetCullMode(_vk_command_buffer, cull_back_faces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE);
    }
    // set mesh vertices
    std::vector<VkBuffer> vertex_buffers = {mesh->_buffer->_vk_buffer};
    std::vector<VkDeviceSize> offsets(vertex_buffers.size(), mesh->_offset);
    vkCmdBindVertexBuffers(_vk_command_buffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());
    // set shader parameters
    VkPushConstantRange mesh_range = shader->_push_constants.at("params");
    DrawParameters params = {std::get<0>(mesh_coordinates_in_camera).to_vec4(),
                             Matrix(std::get<1>(mesh_coordinates_in_camera).inverse()).to_mat3(),
                             vec4({camera.aperture_width, (camera.aperture_width*height)/width, camera.focal_length, camera.max_distance}),
                             static_cast<uint32_t>(camera.projection_type),
                             static_cast<float>(std::get<2>(mesh_coordinates_in_camera))};
    vkCmdPushConstants(_vk_command_buffer, shader->_vk_pipeline_layout, mesh_range.stageFlags, mesh_range.offset, mesh_range.size, &params);
    // send a command to command buffer
    vkCmdDraw(_vk_command_buffer, mesh->bytes_size()/sizeof(Vertex), 1, 0, 0);
    // register layout transitions
    for (std::pair<std::string, VkImageLayout> layout : shader->_final_layouts)
    {
        images.at(layout.first)->_current_layout = layout.second;
    }
}


void Canvas::light(const Camera& camera, const Light& light, const std::tuple<Vector, Quaternion, double>& light_coordinates_in_camera, Canvas* shadow_map)
{
    _record_commands();
    Shader* shader = gpu->_shaders.at("Light");
    std::map<const std::string, Image*> images_pool(
        {{"shadow_map", (shadow_map == nullptr) ? gpu->_default_textures[0].get() : shadow_map->images.at("depth")},
         {"depth", images.at("depth")},
         {"color", images.at("color")},
         {"material", images.at("material")},
         {"normal", images.at("normal")},
         {"albedo", images.at("albedo")}});
    _bind_shader(shader, images_pool);
    if (shadow_map != nullptr)
    {
        _dependencies.insert(shadow_map);
    }
    // bind descriptor sets
    _bind_descriptor_set(shader, 0, images_pool, {});
    // set mesh scale/position/rotation
    uint32_t shadow_map_height = (shadow_map == nullptr) ? 1 : shadow_map->height;
    uint32_t shadow_map_width = (shadow_map == nullptr) ? 1 : shadow_map->width;
    VkPushConstantRange push_range = shader->_push_constants.at("params");
    LightParameters light_parameters = {std::get<0>(light_coordinates_in_camera).to_vec4(),
                                        Matrix(std::get<1>(light_coordinates_in_camera).inverse()).to_mat3(),
                                        vec4({light.color.r, light.color.g, light.color.b, light.intensity}),
                                        vec4({light.aperture_width, (light.aperture_width*shadow_map_height)/shadow_map_width, light.focal_length, light.max_distance}),
                                        vec4({camera.aperture_width, (camera.aperture_width*height)/width, camera.focal_length, camera.max_distance}),
                                        static_cast<uint32_t>(light.projection_type),
                                        static_cast<uint32_t>(camera.projection_type),
                                        camera.sensitivity,
                                        static_cast<uint32_t>((shadow_map == nullptr) ? 0 : 1)};
    vkCmdPushConstants(_vk_command_buffer, shader->_vk_pipeline_layout, push_range.stageFlags, push_range.offset, push_range.size, &light_parameters);
    // send a command to command buffer
    vkCmdDraw(_vk_command_buffer, 6, 1, 0, 0);
    // register layout transitions
    for (std::pair<std::string, VkImageLayout> layout : shader->_final_layouts)
    {
        images.at(layout.first)->_current_layout = layout.second;
    }
}


void Canvas::render()
{
    // if nothing new to render, exit
    if (_rendering)
    {
        return;
    }
    if (!_recording)
    {
        return;
    }
    // End render pass
    _bind_shader(nullptr, images);
    // Transition color to present or transfer dest layout
    _command_barrier({{"color", _final_layout}}, images);
    // End command buffer
    if (vkEndCommandBuffer(_vk_command_buffer) != VK_SUCCESS)
    {
        THROW_ERROR("failed to record command buffer!");
    }
    // list semaphores to wait
    std::vector<VkSemaphore> wait_semaphores;
    std::vector<VkPipelineStageFlags> wait_stages;
    for (const Canvas* canvas : _dependencies)
    {
        wait_semaphores.push_back(canvas->_vk_rendered_semaphore);
        wait_stages.push_back(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    }
    for (VkSemaphore semaphore : _wait_semaphores)
    {
        wait_semaphores.push_back(semaphore);
        wait_stages.push_back(VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
    }
    // submit graphic commands
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = wait_semaphores.size();
    submitInfo.pWaitSemaphores = wait_semaphores.data();
    submitInfo.pWaitDstStageMask = wait_stages.data();
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_vk_command_buffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &_vk_rendered_semaphore;
    if (vkQueueSubmit(std::get<1>(gpu->_graphics_queue.value()), 1, &submitInfo, _vk_fence) != VK_SUCCESS)
    {
        THROW_ERROR("failed to submit draw command buffer!");
    }
    // set the rendering flag
    _recording = false;
    _rendering = true;
    // reset dependencies
    _dependencies.clear();
    _wait_semaphores.clear();
}


VkFramebuffer Canvas::_allocate_frame_buffer(const Shader* shader)
{
    VkFramebuffer frame_buffer;
    std::vector<VkImageView> attachments;
    for (const std::pair<const std::string, VkFormat>& image : shader->_output_attachments)
    {
        attachments.push_back(images.at(image.first)->_vk_image_view);
    }
    if (shader->_depth_test)
    {
        attachments.push_back(images.at("depth")->_vk_image_view);
    }
    VkFramebufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = shader->_vk_render_pass;
    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.data();
    info.width = width;
    info.height = height;
    info.layers = 1;
    if (vkCreateFramebuffer(gpu->_logical_device, &info, nullptr, &frame_buffer) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create framebuffer");
    }
    return frame_buffer;
}


void Canvas::_allocate_command_buffer(VkCommandBuffer& command_buffer, VkCommandPool pool)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = std::get<2>(gpu->_graphics_queue.value());
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(gpu->_logical_device, &allocInfo, &command_buffer) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate command buffers!");
    }
}


void Canvas::_allocate_fence(VkFence& fence)
{
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;//VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(gpu->_logical_device, &fenceInfo, nullptr, &fence) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create VkFence");
    }
}


void Canvas::_allocate_semaphore(VkSemaphore& semaphore)
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(gpu->_logical_device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create VkSemaphore");
    }
}


void Canvas::wait_completion()
{
    if (_rendering)
    {
        vkWaitForFences(gpu->_logical_device, 1, &_vk_fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(gpu->_logical_device, 1, &_vk_fence);
        _rendering = false;
    }
}


bool Canvas::is_recording() const
{
    return _recording;
}


bool Canvas::is_rendering() const
{
    return _rendering;
}


void Canvas::_record_commands()
{
    wait_completion();
    if (!_recording)
    {
        // reset command buffer
        vkResetCommandBuffer(_vk_command_buffer, 0);
        // begin command buffer recording
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional
        if (vkBeginCommandBuffer(_vk_command_buffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(width);
        viewport.height = static_cast<float>(height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(_vk_command_buffer, 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {width, height};
        vkCmdSetScissor(_vk_command_buffer, 0, 1, &scissor);
        // setup the recording flag
        _recording = true;
    }
}


void Canvas::_bind_shader(const Shader* shader, const std::map<const std::string, Image*>& images_pool)
{
    // read layout transitions that have to be performed
    std::map<std::string, VkImageLayout> layout_transitions;
    if (shader != nullptr)
    {
        for (const std::pair<std::string, VkFormat>& image : shader->_output_attachments)
        {
            layout_transitions[image.first] = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        for (const std::map<std::string, VkDescriptorSetLayoutBinding>& set : shader->_descriptor_sets)
        {
            for (const std::pair<std::string, VkDescriptorSetLayoutBinding>& descriptor : set)
            {
                if (descriptor.second.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
                {
                    std::map<std::string, Image*>::const_iterator img = images_pool.find(descriptor.first);
                    if (img != images_pool.end())
                    {
                        layout_transitions[img->first] = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    }
                }
            }
        }
        if (shader->_depth_test)
        {
            layout_transitions["depth"] = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
    }
    // end previous render pass
    bool ended_render_pass = false;
    if (shader != _current_shader || layout_transitions.size() > 0)
    {
        if (_current_shader != nullptr)
        {
            if (_current_shader->_vk_render_pass != VK_NULL_HANDLE)
            {
                vkCmdEndRenderPass(_vk_command_buffer);
                ended_render_pass = true;
            }
        }
    }
    // set a command barrier to transition image layouts
    if (layout_transitions.size() > 0)
    {
        // apply command barrier
        _command_barrier(layout_transitions, images_pool);
    }
    // start new shader's render pass
    if ((_current_shader == nullptr || ended_render_pass) && shader != nullptr)
    {
        // bind new shader pipeline
        vkCmdBindPipeline(_vk_command_buffer, shader->_vk_pipeline_bind_point, shader->_vk_pipeline);
        if (shader->_vk_render_pass != VK_NULL_HANDLE)
        {
            // clear values are used only for attachments with loadOp VK_ATTACHMENT_LOAD_OP_CLEAR
            std::vector<VkClearValue> clear_values;
            for (unsigned int i = 0; i < shader->_output_attachments.size() + 1; i++)  // +1 is for the depth buffer attachment
            {
                VkClearValue clear_value = {};
                clear_value.color = { {0.0f, 0.0f, 0.0f, 0.0f} };
                clear_value.depthStencil = { 1.0f, 0 };
                clear_values.push_back(clear_value);
            }
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = shader->_vk_render_pass;
            renderPassInfo.framebuffer = _frame_buffers.at(shader);
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = { width, height };
            renderPassInfo.clearValueCount = clear_values.size();
            renderPassInfo.pClearValues = clear_values.data();
            vkCmdBeginRenderPass(_vk_command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        }
    }
    // set new shader pointer
    _current_shader = shader;
}


void Canvas::_bind_descriptor_set(const Shader* shader,
    unsigned int descriptor_set_index,
    const std::map<const std::string, Image*>& images_pool,
    const std::map<const std::string, Buffer*>& buffers_pool)
{
    std::vector<VkWriteDescriptorSet> descriptors;
    std::deque<VkDescriptorBufferInfo> buffers;
    std::deque<VkDescriptorImageInfo> samplers;
    for (const std::pair<std::string, VkDescriptorSetLayoutBinding>& descriptor : shader->_descriptor_sets[descriptor_set_index])
    {
        VkWriteDescriptorSet descriptor_set_binding{};
        descriptor_set_binding.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_set_binding.dstSet = VK_NULL_HANDLE;
        descriptor_set_binding.dstBinding = descriptor.second.binding;
        descriptor_set_binding.descriptorType = descriptor.second.descriptorType;
        descriptor_set_binding.descriptorCount = descriptor.second.descriptorCount;
        descriptor_set_binding.pBufferInfo = nullptr;
        descriptor_set_binding.pImageInfo = nullptr;
        if (descriptor.second.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
        {
            Image* image = images_pool.at(descriptor.first);
            samplers.push_back({image->_vk_sampler, image->_vk_image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL});
            descriptor_set_binding.pImageInfo = &samplers.back();
        }
        else if (descriptor.second.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
        {
            Buffer* buffer = buffers_pool.at(descriptor.first);
            buffers.push_back({buffer->_vk_buffer, 0, VK_WHOLE_SIZE});
            descriptor_set_binding.pBufferInfo = &buffers.back();
        }
        else
        {
            THROW_ERROR("Unexpected descriptor type code : " + std::to_string(descriptor.second.descriptorType));
        }
        descriptors.push_back(descriptor_set_binding);
    }
    vkCmdPushDescriptorSet(_vk_command_buffer, shader->_vk_pipeline_bind_point, shader->_vk_pipeline_layout, 0, descriptors.size(), descriptors.data());
}


void Canvas::_command_barrier(const std::map<std::string, VkImageLayout>& new_image_layouts, const std::map<const std::string, Image*>& images_pool)
{
    std::vector<VkImageMemoryBarrier> layout_transitions;
    VkPipelineStageFlags source_stage = 0;
    VkPipelineStageFlags destination_stage = 0;
    for (const std::pair<std::string, VkImageLayout>& new_image_layout : new_image_layouts)
    {
        Image* image = images_pool.at(new_image_layout.first);
        if (image->_current_layout != new_image_layout.second)
        {
            VkPipelineStageFlagBits source_stage_bits;
            VkPipelineStageFlagBits destination_stage_bits;
            VkImageMemoryBarrier transition{};
            transition.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            transition.oldLayout = image->_current_layout;
            transition.newLayout = new_image_layout.second;
            transition.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            transition.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            transition.image = image->_vk_image;
            transition.subresourceRange.aspectMask = image->_get_aspect_mask();
            transition.subresourceRange.baseMipLevel = 0;
            transition.subresourceRange.levelCount = image->_mip_levels;
            transition.subresourceRange.baseArrayLayer = 0;
            transition.subresourceRange.layerCount = 1;
            transition.srcAccessMask = 0;  // will be modified later in the function
            transition.dstAccessMask = 0;  // will be modified later in the function
            std::tie(transition.srcAccessMask, source_stage_bits) = Image::_source_layout_attributes(image->_current_layout);
            std::tie(transition.dstAccessMask, destination_stage_bits) = Image::_destination_layout_attributes(new_image_layout.second);
            source_stage = source_stage | source_stage_bits;
            destination_stage = destination_stage | destination_stage_bits;
            layout_transitions.push_back(transition);
            image->_current_layout = new_image_layout.second;
        }
    }
    if (source_stage == 0 && destination_stage == 0)
    {
        source_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    vkCmdPipelineBarrier(
        _vk_command_buffer,
        source_stage, destination_stage,
        0, // VK_DEPENDENCY_BY_REGION_BIT
        0, nullptr,
        0, nullptr,
        layout_transitions.size(), layout_transitions.data()
    );
}


// void Canvas::_allocate_camera_view(Buffer*& camera_view)
// {
//     camera_view = new Buffer(gpu, sizeof(CameraParameters),
//                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

// }


// void Canvas::set_view(const Camera& camera)
// {
//     if (_rendering)
//     {
//         wait_completion();
//     }
//     if (!_recording)
//     {
//         _initialize_recording();
//     }
//     if (_recording_render_pass)
//     {
//         _end_render_pass_recording();
//     }
//     // Read camera view
//     std::tuple<Vector, Quaternion, double> camera_coordinates = camera.absolute_coordinates();
//     CameraParameters params{};
//     params.camera_position = std::get<0>(camera_coordinates).to_vec4();
//     params.world_to_camera = Matrix(std::get<1>(camera_coordinates)).to_mat3();
//     params.camera_scale = static_cast<float>(std::get<2>(camera_coordinates));
//     params.focal_length = camera.focal_length();
//     params.camera_aperture_size = {0., 0.};
//     _camera_view->upload(reinterpret_cast<uint8_t*>(&params), sizeof(CameraParameters), 0);
//     // Push camera view to device
//     VkDescriptorBufferInfo camera_parameters = {_camera_view->_vk_buffer, 0, VK_WHOLE_SIZE};
//     VkWriteDescriptorSet uniform_buffer{};
//     uniform_buffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     uniform_buffer.dstBinding = 0;
//     uniform_buffer.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     uniform_buffer.descriptorCount = 1;
// 	   uniform_buffer.pBufferInfo = &camera_parameters;
//     std::vector<VkWriteDescriptorSet> descriptors = {uniform_buffer};
//     vkCmdPushDescriptorSet(_vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpu->shader3d->_pipeline_layout, 0, descriptors.size(), descriptors.data());
// }
