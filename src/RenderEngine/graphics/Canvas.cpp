#include <RenderEngine/render_engine.hpp>
#include <RenderEngine/graphics/Canvas.hpp>
#include <RenderEngine/graphics/model/Mesh.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>
using namespace RenderEngine;

Canvas::Canvas(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height, bool mip_maped, AntiAliasing sample_count) :
    gpu(_gpu),
    color(_gpu, ImageFormat::RGBA, width, height, mip_maped),
    normal(_gpu, ImageFormat::RGBA, width, height, mip_maped),
    material(_gpu, ImageFormat::RGBA, width, height, mip_maped),
    depth_buffer(_gpu, ImageFormat::DEPTH, width, height, false)
{
    _allocate_frame_buffer();
    _allocate_command_buffer(_vk_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    _allocate_fence(_vk_fence);
    _allocate_semaphore(_rendered_semaphore);
    _allocate_camera_view(_camera_view);
}


Canvas::Canvas(const std::shared_ptr<GPU>& _gpu, const VkImage& vk_image, uint32_t width, uint32_t height, AntiAliasing sample_count) :
    gpu(_gpu),
    color(_gpu, vk_image, nullptr, ImageFormat::RGBA, width, height, false),
    normal(_gpu, ImageFormat::RGBA, width, height, false),
    material(_gpu, ImageFormat::RGBA, width, height, false),
    depth_buffer(_gpu, ImageFormat::DEPTH, width, height, false)
{
    _allocate_frame_buffer();
    _allocate_command_buffer(_vk_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    _allocate_fence(_vk_fence);
    _allocate_semaphore(_rendered_semaphore);
    _allocate_camera_view(_camera_view);
}


Canvas::~Canvas()
{
    vkDeviceWaitIdle(gpu->_logical_device);
    vkDestroySemaphore(gpu->_logical_device, _rendered_semaphore, nullptr);
    vkDestroyFence(gpu->_logical_device, _vk_fence, nullptr);
    vkFreeCommandBuffers(gpu->_logical_device, std::get<2>(gpu->_graphics_queue.value()), 1, &_vk_command_buffer);
    vkDestroyFramebuffer(gpu->_logical_device, _vk_frame_buffer, nullptr);
    delete _camera_view;
}


void Canvas::_allocate_frame_buffer()
{
    std::shared_ptr<GPU>& _gpu = this->gpu;
    std::vector<VkImageView> attachments = {color._vk_image_view, normal._vk_image_view, material._vk_image_view, depth_buffer._vk_image_view};
    VkFramebufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = gpu->shader3d->_render_pass;
    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.data();
    info.width = color.width();
    info.height = color.height();
    info.layers = 1;
    if (vkCreateFramebuffer(gpu->_logical_device, &info, nullptr, &_vk_frame_buffer) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create framebuffer");
    }
}


void Canvas::_allocate_command_buffer(VkCommandBuffer& command_buffer, VkCommandPool pool)
{
    std::shared_ptr<GPU>& _gpu = this->gpu;
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
    std::shared_ptr<GPU>& _gpu = this->gpu;
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
    std::shared_ptr<GPU>& _gpu = this->gpu;
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(gpu->_logical_device, &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create VkSemaphore");
    }
}


void Canvas::_allocate_camera_view(Buffer*& camera_view)
{
    camera_view = new Buffer(gpu, sizeof(CameraParameters),
                             VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);

}


// void Canvas::_allocate_descriptor_sets(std::array<std::shared_ptr<VkDescriptorSet>, 1>& descriptor_sets)
// {
//     for (unsigned int i=0;i<_descriptor_sets.size();i++)
//     {
//         VkDescriptorSetAllocateInfo allocInfo{};
//         allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//         allocInfo.descriptorPool = descriptorPool;
//         allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
//         allocInfo.pSetLayouts = layouts.data();
//         if (vkAllocateDescriptorSets(gpu->_logical_device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
//         {
//             THROW_ERROR("Failed to create descriptor set " + std::to_string(i));
//         }
//     }
// }


// void Canvas::_deallocate_frame_buffer(const std::shared_ptr<GPU>& gpu, VkFramebuffer* frame_buffer)
// {
//     vkDestroyFramebuffer(gpu->_logical_device, *frame_buffer, nullptr);
//     *frame_buffer = VK_NULL_HANDLE;
// }


// void Canvas::_deallocate_command_buffer(const std::shared_ptr<GPU>& gpu, const VkCommandPool& pool, VkCommandBuffer* command_buffer)
// {
//     vkFreeCommandBuffers(gpu->_logical_device, pool, 1, command_buffer);
//     *command_buffer = VK_NULL_HANDLE;
// }


// void Canvas::_deallocate_fence(const std::shared_ptr<GPU>& gpu, VkFence* fence)
// {
//     vkDestroyFence(gpu->_logical_device, *fence, nullptr);
//     *fence = VK_NULL_HANDLE;
// }


// void Canvas::_deallocate_semaphore(const std::shared_ptr<GPU>& gpu, VkSemaphore* semaphore)
// {
//     vkDestroySemaphore(gpu->_logical_device, *semaphore, nullptr);
//     *semaphore = VK_NULL_HANDLE;
// }


void Canvas::_initialize_recording()
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
    // bind the successive pipelines
    for (unsigned int i = 0; i < gpu->shader3d->_pipelines.size(); i++)
    {
        vkCmdBindPipeline(_vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpu->shader3d->_pipelines[i]);
    }
    // set viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(color.width());
    viewport.height = static_cast<float>(color.height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(_vk_command_buffer, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { color.width(), color.height() };
    vkCmdSetScissor(_vk_command_buffer, 0, 1, &scissor);
    // setup the recording flag
    _recording = true;
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


void Canvas::clear(unsigned char R, unsigned char G, unsigned char B, unsigned char A)
{
    if (_rendering)
    {
        wait_completion();
    }
    if (!_recording)
    {
        _initialize_recording();
    }
    if (_recording_render_pass)
    {
        _end_render_pass_recording();
    }
    // transition layout
    color._transition_to_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _vk_command_buffer);
    normal._transition_to_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _vk_command_buffer);
    material._transition_to_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _vk_command_buffer);
    depth_buffer._transition_to_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, _vk_command_buffer);
    // Clear albedo image
    VkClearColorValue clear_color = {{R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f}};
    VkImageSubresourceRange color_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, color.mip_levels_count(), 0, 1};
    vkCmdClearColorImage(_vk_command_buffer, color._vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &color_range);
    // Clear normal image
    VkClearColorValue clear_normal = {{0.f, 0.f, 0.f, 1.0f}};
    VkImageSubresourceRange normal_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, color.mip_levels_count(), 0, 1};
    vkCmdClearColorImage(_vk_command_buffer, normal._vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_normal, 1, &normal_range);
    // Clear material image
    VkClearColorValue clear_material = {{0.f, 0.f, 0.f, 1.0f}};
    VkImageSubresourceRange material_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, color.mip_levels_count(), 0, 1};
    vkCmdClearColorImage(_vk_command_buffer, material._vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_material, 1, &material_range);
    // Clear depth buffer
    VkClearDepthStencilValue clear_depth = {0.f, 0};
    VkImageSubresourceRange depth_range = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, color.mip_levels_count(), 0, 1};
    vkCmdClearDepthStencilImage(_vk_command_buffer, depth_buffer._vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_depth, 1, &depth_range);
}

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
// 	uniform_buffer.pBufferInfo = &camera_parameters;
//     std::vector<VkWriteDescriptorSet> descriptors = {uniform_buffer};
//     vkCmdPushDescriptorSet(_vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpu->shader3d->_pipeline_layouts[0], 0, descriptors.size(), descriptors.data());
// }

void Canvas::draw(const Camera& camera, const std::shared_ptr<Mesh>& mesh, const std::tuple<Vector, Quaternion, double>& coordinates_in_camera, bool cull_back_faces)
{
    if (_rendering)
    {
        wait_completion();
    }
    if (!_recording)
    {
        _initialize_recording();
    }
    if (!_recording_render_pass)
    {
        _start_render_pass_recording();
    }
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
    VkPushConstantRange mesh_range = gpu->shader3d->_push_constants[0][0].second;
    MeshDrawParameters mesh_parameters = {std::get<0>(coordinates_in_camera).to_vec4(),
                                          Matrix(std::get<1>(coordinates_in_camera).inverse()).to_mat3(),
                                          vec4({camera.field_of_view, static_cast<float>(color.width())/color.height(), camera.near_plane, camera.far_plane}),
                                          static_cast<float>(std::get<2>(coordinates_in_camera))};
    vkCmdPushConstants(_vk_command_buffer, gpu->shader3d->_pipeline_layouts[0], mesh_range.stageFlags, mesh_range.offset, mesh_range.size, &mesh_parameters);
    // send a command to command buffer
    vkCmdDraw(_vk_command_buffer, mesh->bytes_size()/sizeof(Vertex), 1, 0, 0);
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
    // list semaphores to wait
    std::vector<VkSemaphore> wait_semaphores;
    for (const VkSemaphore& semaphore : _dependencies)
    {
        wait_semaphores.push_back(semaphore);
    }
    // End render pass
    if (_recording_render_pass)
    {
        _end_render_pass_recording();
    }
    // End command buffer
    if (vkEndCommandBuffer(_vk_command_buffer) != VK_SUCCESS)
    {
        THROW_ERROR("failed to record command buffer!");
    }
    // submit graphic commands
    VkPipelineStageFlags wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = wait_semaphores.size();
    submitInfo.pWaitSemaphores = wait_semaphores.data();
    submitInfo.pWaitDstStageMask = &wait_stages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_vk_command_buffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &_rendered_semaphore;
    if (vkQueueSubmit(std::get<1>(gpu->_graphics_queue.value()), 1, &submitInfo, _vk_fence) != VK_SUCCESS)
    {
        THROW_ERROR("failed to submit draw command buffer!");
    }
    // set the rendering flag
    _recording = false;
    _rendering = true;
    // reset dependencies
    _dependencies.clear();
}

bool Canvas::is_recording() const
{
    return _recording;
}

bool Canvas::is_rendering() const
{
    return _rendering;
}

void Canvas::_start_render_pass_recording()
{
    // transition layouts
    color._transition_to_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, _vk_command_buffer);
    normal._transition_to_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, _vk_command_buffer);
    material._transition_to_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, _vk_command_buffer);
    depth_buffer._transition_to_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, _vk_command_buffer);
    // begin render pass
    std::vector<VkClearValue> clear_values;
    for (unsigned int i = 0; i < 4; i++)
    {
        VkClearValue clear_value = {};
        clear_value.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clear_value.depthStencil = { 1.0f, 0 };
        clear_values.push_back(clear_value);
    }
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = gpu->shader3d->_render_pass;
    renderPassInfo.framebuffer = _vk_frame_buffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { color.width(), color.height() };
    renderPassInfo.clearValueCount = clear_values.size();
    renderPassInfo.pClearValues = clear_values.data();
    vkCmdBeginRenderPass(_vk_command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    // Set the flag
    _recording_render_pass = true;
}


void Canvas::_end_render_pass_recording()
{
    vkCmdEndRenderPass(_vk_command_buffer);
    _recording_render_pass = false;
}