#include <RenderEngine/render_engine.hpp>
#include <RenderEngine/graphics/Canvas.hpp>
#include <RenderEngine/graphics/model/Mesh.hpp>
#include <RenderEngine/graphics/shaders/PushConstants.hpp>
using namespace RenderEngine;

Canvas::Canvas(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height, Image::AntiAliasing sample_count, bool texture_compatible) :
    gpu(_gpu),
    color(_gpu, width, height, ImageFormat::RGBA, sample_count, texture_compatible),
    handles(_gpu, width, height, ImageFormat::POINTER, Image::AntiAliasing::X1, false, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    depth_buffer(_gpu, width, height, ImageFormat::DEPTH, Image::AntiAliasing::X1, false, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
{
    allocate_frame_buffer();
    allocate_command_buffer(_draw_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    // allocate_command_buffer(_transfer_command_buffer, std::get<2>(gpu->_transfer_queue.value()));
    allocate_fence(_rendered_fence);
    // allocate_fence(_transfered_fence);
    allocate_semaphore(_rendered_semaphore);
    // allocate_semaphore(_transfered_semaphore);
}


Canvas::Canvas(const std::shared_ptr<GPU>& _gpu, const std::shared_ptr<VkImage>& vk_image, uint32_t width, uint32_t height, Image::AntiAliasing sample_count, bool texture_compatible) :
    gpu(_gpu),
    color(_gpu, vk_image, width, height, ImageFormat::RGBA, sample_count, texture_compatible),
    handles(_gpu, width, height, ImageFormat::POINTER, Image::AntiAliasing::X1, false, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    depth_buffer(_gpu, width, height, ImageFormat::DEPTH, Image::AntiAliasing::X1, false, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
{
    allocate_frame_buffer();
    allocate_command_buffer(_draw_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    // allocate_command_buffer(_transfer_command_buffer, std::get<2>(gpu->_transfer_queue.value()));
    allocate_fence(_rendered_fence);
    // allocate_fence(_transfered_fence);
    allocate_semaphore(_rendered_semaphore);
    // allocate_semaphore(_transfered_semaphore);
}


Canvas::~Canvas()
{
    vkDeviceWaitIdle(gpu->_logical_device);
}


void Canvas::allocate_frame_buffer()
{
    std::shared_ptr<GPU>& _gpu = this->gpu;
    _frame_buffer.reset(new VkFramebuffer, [_gpu](VkFramebuffer* frm_buffer) {Canvas::_deallocate_frame_buffer(_gpu, frm_buffer);});
    std::vector<VkImageView> attachments = {*color._vk_image_view, *depth_buffer._vk_image_view};
    VkFramebufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = gpu->shader3d->_render_pass;
    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.data();
    info.width = color.width();
    info.height = color.height();
    info.layers = 1;
    if (vkCreateFramebuffer(gpu->_logical_device, &info, nullptr, _frame_buffer.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create framebuffer");
    }
}


void Canvas::allocate_command_buffer(std::shared_ptr<VkCommandBuffer>& command_buffer, VkCommandPool pool)
{
    std::shared_ptr<GPU>& _gpu = this->gpu;
    command_buffer.reset(new VkCommandBuffer, [_gpu, pool](VkCommandBuffer* cmd_buffer) {Canvas::_deallocate_command_buffer(_gpu, pool, cmd_buffer);});
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = std::get<2>(gpu->_graphics_queue.value());
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(gpu->_logical_device, &allocInfo, command_buffer.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to allocate command buffers!");
    }
}


void Canvas::allocate_fence(std::shared_ptr<VkFence>& fence)
{
    std::shared_ptr<GPU>& _gpu = this->gpu;
    fence.reset(new VkFence, [_gpu](VkFence* fnc) {Canvas::_deallocate_fence(_gpu, fnc);});
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;//VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(gpu->_logical_device, &fenceInfo, nullptr, fence.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create VkFence");
    }
}


void Canvas::allocate_semaphore(std::shared_ptr<VkSemaphore>& semaphore)
{
    std::shared_ptr<GPU>& _gpu = this->gpu;
    semaphore.reset(new VkSemaphore, [_gpu](VkSemaphore* smp) {Canvas::_deallocate_semaphore(_gpu, smp);});
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(gpu->_logical_device, &semaphoreInfo, nullptr, semaphore.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create VkSemaphore");
    }
}


void Canvas::_deallocate_frame_buffer(const std::shared_ptr<GPU>& gpu, VkFramebuffer* frame_buffer)
{
    vkDestroyFramebuffer(gpu->_logical_device, *frame_buffer, nullptr);
    *frame_buffer = VK_NULL_HANDLE;
}


void Canvas::_deallocate_command_buffer(const std::shared_ptr<GPU>& gpu, const VkCommandPool& pool, VkCommandBuffer* command_buffer)
{
    vkFreeCommandBuffers(gpu->_logical_device, pool, 1, command_buffer);
    *command_buffer = VK_NULL_HANDLE;
}


void Canvas::_deallocate_fence(const std::shared_ptr<GPU>& gpu, VkFence* fence)
{
    vkDestroyFence(gpu->_logical_device, *fence, nullptr);
    *fence = VK_NULL_HANDLE;
}


void Canvas::_deallocate_semaphore(const std::shared_ptr<GPU>& gpu, VkSemaphore* semaphore)
{
    vkDestroySemaphore(gpu->_logical_device, *semaphore, nullptr);
    *semaphore = VK_NULL_HANDLE;
}


void Canvas::_initialize_recording()
{
    // reset command buffer
    vkResetCommandBuffer(*_draw_command_buffer, 0);
    // begin command buffer recording
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional
    if (vkBeginCommandBuffer(*_draw_command_buffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    // bind the successive pipelines
    for (unsigned int i = 0; i < gpu->shader3d->_pipelines.size(); i++)
    {
        vkCmdBindPipeline(*_draw_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpu->shader3d->_pipelines[i]);
        //vkCmdBindDescriptorSets(*_draw_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpu->shader3d->_pipeline_layouts[i], 0, 1, &descriptorSets.scene, 0, nullptr);
    }
    // set viewport and scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(color.width());
    viewport.height = static_cast<float>(color.height());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(*_draw_command_buffer, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = { color.width(), color.height() };
    vkCmdSetScissor(*_draw_command_buffer, 0, 1, &scissor);
    // setup the recording flag
    _recording = true;
}


void Canvas::wait_completion()
{
    if (_rendering)
    {
        vkWaitForFences(gpu->_logical_device, 1, _rendered_fence.get(), VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(gpu->_logical_device, 1, _rendered_fence.get());
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
    // end eventual render pass
    _end_render_pass();
    // transition layout
    color._transition_to_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, *_draw_command_buffer);
    depth_buffer._transition_to_layout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, *_draw_command_buffer);
    // Clear color image
    VkClearColorValue clear_color = {{R / 255.0f, G / 255.0f, B / 255.0f, A / 255.0f}};
    VkImageSubresourceRange color_range = {VK_IMAGE_ASPECT_COLOR_BIT, 0, color._mip_levels, 0, 1};
    vkCmdClearColorImage(*_draw_command_buffer, *color._vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &color_range);
    // Clear depth buffer
    VkClearDepthStencilValue clear_depth = {0.f, 0};
    VkImageSubresourceRange depth_range = {VK_IMAGE_ASPECT_DEPTH_BIT, 0, color._mip_levels, 0, 1};
    vkCmdClearDepthStencilImage(*_draw_command_buffer, *depth_buffer._vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_depth, 1, &depth_range);
}


void Canvas::draw(const Mesh& mesh, const Matrix& mesh_rotation)
{
    if (_rendering)
    {
        wait_completion();
    }
    if (!_recording)
    {
        _initialize_recording();
    }
    // transition layouts
    color._transition_to_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, *_draw_command_buffer);
    depth_buffer._transition_to_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, *_draw_command_buffer);
    // start render pass
    _start_render_pass();
    // draw
    std::vector<VkBuffer> vertex_buffers = {*mesh._vk_buffer};
    std::vector<VkDeviceSize> offsets(vertex_buffers.size(), 0);
    vkCmdBindVertexBuffers(*_draw_command_buffer, 0, vertex_buffers.size(), vertex_buffers.data(), offsets.data());
    VkPushConstantRange range = gpu->shader3d->_push_constants[0][0].second;
    PushConstants constants = {{0., 0., 0.5}, {1., 1., 1.}, mesh_rotation.to_mat3()};
    vkCmdPushConstants(*_draw_command_buffer, gpu->shader3d->_pipeline_layouts[0], range.stageFlags, range.offset, range.size, &constants);
    // send a command to command buffer
    vkCmdDraw(*_draw_command_buffer, mesh.size/sizeof(Vertex), 1, 0, 0);
}


void Canvas::_start_render_pass()
{
    if (!_in_render_pass)
    {
        // begin render pass
        std::vector<VkClearValue> clear_values;
        for (unsigned int i = 0; i < 3; i++)
        {
            VkClearValue clear_value = {};
            clear_value.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
            clear_value.depthStencil = { 1.0f, 0 };
            clear_values.push_back(clear_value);
        }
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = gpu->shader3d->_render_pass;
        renderPassInfo.framebuffer = *_frame_buffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { color.width(), color.height() };
        renderPassInfo.clearValueCount = clear_values.size();
        renderPassInfo.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(*_draw_command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        // Set the flag
        _in_render_pass = true;
    }
}


void Canvas::_end_render_pass()
{
    if (_in_render_pass)
    {
        vkCmdEndRenderPass(*_draw_command_buffer);
        _in_render_pass = false;
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
    // list semaphores to wait
    std::vector<VkSemaphore> wait_semaphores;
    for (const VkSemaphore& semaphore : _dependencies)
    {
        wait_semaphores.push_back(semaphore);
    }
    // End render pass
    _end_render_pass();
    // End command buffer
    if (vkEndCommandBuffer(*_draw_command_buffer) != VK_SUCCESS)
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
    submitInfo.pCommandBuffers = _draw_command_buffer.get();
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = _rendered_semaphore.get();
    if (vkQueueSubmit(std::get<1>(gpu->_graphics_queue.value()), 1, &submitInfo, *_rendered_fence) != VK_SUCCESS)
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