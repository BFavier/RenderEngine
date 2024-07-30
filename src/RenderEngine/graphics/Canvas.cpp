#include <RenderEngine/render_engine.hpp>
#include <RenderEngine/graphics/Canvas.hpp>
#include <RenderEngine/graphics/model/Mesh.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>
#include <RenderEngine/utilities/Macro.hpp>
using namespace RenderEngine;


Canvas::Canvas(const std::shared_ptr<GPU>& _gpu, uint32_t _width, uint32_t _height, bool mip_maped, AntiAliasing sample_count) :
    gpu(_gpu),
    images({{"color", std::make_shared<Image>(_gpu, ImageFormat::RGBA, _width, _height, mip_maped)},
            {"normal", std::make_shared<Image>(_gpu, ImageFormat::NORMAL, _width, _height, mip_maped)},
            {"material", std::make_shared<Image>(_gpu, ImageFormat::MATERIAL, _width, _height, mip_maped)},
            {"depth", std::make_shared<Image>(_gpu, ImageFormat::DEPTH, _width, _height, false)}}),
    width(_width), height(_height), _final_layout(VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL)
{
    for (std::pair<std::string, Shader*> shader : gpu->_shaders)
    {
        _frame_buffers[shader.second] = _allocate_frame_buffer(shader.second);
    }
    _allocate_command_buffer(_vk_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    _allocate_fence(_vk_fence);
    _allocate_semaphore(_vk_rendered_semaphore);
}


Canvas::Canvas(const std::shared_ptr<GPU>& _gpu, const VkImage& vk_image, uint32_t _width, uint32_t _height, AntiAliasing sample_count) :
    gpu(_gpu),
    images({{"color", std::make_shared<Image>(_gpu, vk_image, nullptr, ImageFormat::RGBA, _width, _height, false)},
            {"normal", std::make_shared<Image>(_gpu, ImageFormat::NORMAL, _width, _height, false)},
            {"material", std::make_shared<Image>(_gpu, ImageFormat::MATERIAL, _width, _height, false)},
            {"depth", std::make_shared<Image>(_gpu, ImageFormat::DEPTH, _width, _height, false)}}),
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
    for (std::pair<const Shader*, VkFramebuffer> frame_buffer : _frame_buffers)
    {
        vkDestroyFramebuffer(gpu->_logical_device, frame_buffer.second, nullptr);
    }
}


void Canvas::clear(Color _color)
{
    wait_completion();
    _record_commands();
    // transition layout
    std::map<std::string, VkImageLayout> transitions;
    for (const std::pair<const std::string, std::shared_ptr<Image>>& image : images)
    {
        transitions[image.first] = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }
    _command_barrier(transitions);
    // clear images
    for (const std::pair<const std::string, std::shared_ptr<Image>>& image : images)
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


void Canvas::draw(const Camera& camera, const std::shared_ptr<Mesh>& mesh, const std::tuple<Vector, Quaternion, double>& coordinates_in_camera, bool cull_back_faces)
{
    wait_completion();
    _record_commands();
    Shader* shader = gpu->_shaders.at("3D");
    _bind_shader(shader);
    // set culling mode
    if (gpu->dynamic_culling_supported())
    {
        vkCmdSetCullMode(_vk_command_buffer, cull_back_faces ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE);
    }
    // set mesh vertices
    std::vector<VkBuffer> vertex_buffers = {mesh->_buffer->_vk_buffer};
    std::vector<VkDeviceSize> offsets(vertex_buffers.size(), mesh->_offset);
    vkCmdBindVertexBuffers(_vk_command_buffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());
    // set mesh scale/position/rotation
    VkPushConstantRange mesh_range = shader->_push_constants.at("params");
    MeshDrawParameters mesh_parameters = {std::get<0>(coordinates_in_camera).to_vec4(),
                                          Matrix(std::get<1>(coordinates_in_camera).inverse()).to_mat3(),
                                          vec4({camera.horizontal_length, camera.horizontal_length*static_cast<float>(height)/width, camera.near_plane_distance, camera.far_plane_distance}),
                                          static_cast<float>(std::get<2>(coordinates_in_camera))};
    vkCmdPushConstants(_vk_command_buffer, shader->_vk_pipeline_layout, mesh_range.stageFlags, mesh_range.offset, mesh_range.size, &mesh_parameters);
    // send a command to command buffer
    vkCmdDraw(_vk_command_buffer, mesh->bytes_size()/sizeof(Vertex), 1, 0, 0);
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
    _bind_shader(nullptr);
    // Transition color to present or transfer dest layout
    _command_barrier({{"color", _final_layout}});
    // End command buffer
    if (vkEndCommandBuffer(_vk_command_buffer) != VK_SUCCESS)
    {
        THROW_ERROR("failed to record command buffer!");
    }
    // list semaphores to wait
    std::vector<VkSemaphore> wait_semaphores;
    for (const Canvas* canvas : _dependencies)
    {
        wait_semaphores.push_back(canvas->_vk_rendered_semaphore);
    }
    for (VkSemaphore semaphore : _wait_semaphores)
    {
        wait_semaphores.push_back(semaphore);
    }
    // submit graphic commands
    VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT | VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = wait_semaphores.size();
    submitInfo.pWaitSemaphores = wait_semaphores.data();
    submitInfo.pWaitDstStageMask = &wait_stages;
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
    for (const std::pair<const std::string, VkFormat>& image : shader->_input_attachments)
    {
        attachments.push_back(images.at(image.first)->_vk_image_view);
    }
    for (const std::pair<const std::string, VkFormat>& image : shader->_output_attachments)
    {
        attachments.push_back(images.at(image.first)->_vk_image_view);
    }
    attachments.push_back(images.at("depth")->_vk_image_view);
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


void Canvas::_bind_shader(const Shader* shader)
{
    if (shader == _current_shader)
    {
        return;
    }
    // end previous render pass
    if (_current_shader != nullptr)
    {
        if (_current_shader->_vk_render_pass != VK_NULL_HANDLE)
        {
            vkCmdEndRenderPass(_vk_command_buffer);
        }
    }
    // transition image layouts for new shader, set a command barrier, and bind the new shader
    if (shader != nullptr)
    {
        // apply command barrier
        std::map<std::string, VkImageLayout> layout_transitions;
        for (const std::pair<std::string, VkFormat>& image : shader->_input_attachments)
        {
            layout_transitions[image.first] = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        for (const std::pair<std::string, VkFormat>& image : shader->_output_attachments)
        {
            layout_transitions[image.first] = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        if (shader->_vk_pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS)
        {
            layout_transitions["depth"] = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        _command_barrier(layout_transitions);
        // bind new shader pipeline
        vkCmdBindPipeline(_vk_command_buffer, shader->_vk_pipeline_bind_point, shader->_vk_pipeline);
        if (shader->_vk_render_pass != VK_NULL_HANDLE)
        {
            // clear values are used only for attachments with loadOp VK_ATTACHMENT_LOAD_OP_CLEAR
            std::vector<VkClearValue> clear_values;
            for (unsigned int i = 0; i < shader->_output_attachments.size() + 1; i++)  // +1 is for the depth buffer attachment
            {
                VkClearValue clear_value = {};
                clear_value.color = {{0.0f, 0.0f, 0.0f, 0.0f}};
                clear_value.depthStencil = {1.0f, 0};
                clear_values.push_back(clear_value);
            }
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = shader->_vk_render_pass;
            renderPassInfo.framebuffer = _frame_buffers.at(shader);
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = {width, height};
            renderPassInfo.clearValueCount = clear_values.size();
            renderPassInfo.pClearValues = clear_values.data();
            vkCmdBeginRenderPass(_vk_command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        }
    }
    // set new shader pointer
    _current_shader = shader;
}


void Canvas::_command_barrier(const std::map<std::string, VkImageLayout>& new_image_layouts)
{
    std::vector<VkImageMemoryBarrier> layout_transitions;
    VkPipelineStageFlags source_stage = 0;
    VkPipelineStageFlags destination_stage = 0;
    for (const std::pair<std::string, VkImageLayout>& new_image_layout : new_image_layouts)
    {
        Image* image = images.at(new_image_layout.first).get();
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
