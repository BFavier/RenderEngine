#include <RenderEngine/render_engine.hpp>
#include <RenderEngine/graphics/Canvas.hpp>
using namespace RenderEngine;

Canvas::Canvas(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height, Image::AntiAliasing sample_count, bool texture_compatible) :
    gpu(_gpu),
    color(_gpu, width, height, ImageFormat::RGBA, sample_count, texture_compatible),
    handles(_gpu, width, height, ImageFormat::POINTER, Image::AntiAliasing::X1, false, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    depth_buffer(_gpu, width, height, ImageFormat::DEPTH, Image::AntiAliasing::X1, false, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
{
    allocate_frame_buffer();
    allocate_command_buffer(_draw_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    // allocate_command_buffer(_fill_command_buffer, std::get<2>(gpu->_transfer_queue.value()));
    allocate_fence();
    allocate_semaphore();
}


Canvas::Canvas(const std::shared_ptr<GPU>& _gpu, const std::shared_ptr<VkImage>& vk_image, uint32_t width, uint32_t height, Image::AntiAliasing sample_count, bool texture_compatible) :
    gpu(_gpu),
    color(_gpu, vk_image, width, height, ImageFormat::RGBA, sample_count, texture_compatible),
    handles(_gpu, width, height, ImageFormat::POINTER, Image::AntiAliasing::X1, false, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),
    depth_buffer(_gpu, width, height, ImageFormat::DEPTH, Image::AntiAliasing::X1, false, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
{
    allocate_frame_buffer();
    allocate_command_buffer(_draw_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    // allocate_command_buffer(_fill_command_buffer, std::get<2>(gpu->_transfer_queue.value()));
    allocate_fence();
    allocate_semaphore();
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


void Canvas::allocate_fence()
{
    std::shared_ptr<GPU>& _gpu = this->gpu;
    _rendered_fence.reset(new VkFence, [_gpu](VkFence* fnc) {Canvas::_deallocate_fence(_gpu, fnc);});
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = 0;//VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(gpu->_logical_device, &fenceInfo, nullptr, _rendered_fence.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create VkFence");
    }
}


void Canvas::allocate_semaphore()
{
    std::shared_ptr<GPU>& _gpu = this->gpu;
    _rendered_semaphore.reset(new VkSemaphore, [_gpu](VkSemaphore* smp) {Canvas::_deallocate_semaphore(_gpu, smp);});
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    if (vkCreateSemaphore(gpu->_logical_device, &semaphoreInfo, nullptr, _rendered_semaphore.get()) != VK_SUCCESS)
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
    // transition layouts
    color._transition_to_layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, *_draw_command_buffer);
    depth_buffer._transition_to_layout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, *_draw_command_buffer);
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
        // wait for previous rendering completion, and reset fence to unsignaled status
        vkWaitForFences(gpu->_logical_device, 1, _rendered_fence.get(), VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(gpu->_logical_device, 1, _rendered_fence.get());
        // reset the rendering flag
        _rendering = false;
    }
}


void Canvas::draw()
{
    // wait until eventual previous rendering ends
    wait_completion();
    if (!_recording)
    {
        _initialize_recording();
    }
    // send a command to command buffer
    vkCmdDraw(*_draw_command_buffer, 3, 1, 0, 0);
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
    // unset the recording flag
    _recording = false;
    // End render pass
    vkCmdEndRenderPass(*_draw_command_buffer);
    // End command buffer
    if (vkEndCommandBuffer(*_draw_command_buffer) != VK_SUCCESS)
    {
        THROW_ERROR("failed to record command buffer!");
    }
    // submit graphic commands
    std::vector<VkSemaphore> wait_semaphores;
    for (const VkSemaphore& semaphore : _dependencies)
    {
        wait_semaphores.push_back(semaphore);
    }
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
    // reset dependencies
    _dependencies.clear();
    // set the rendering flag
    _rendering = true;
}

bool Canvas::is_recording() const
{
    return _recording;
}

bool Canvas::is_rendering() const
{
    return _rendering;
}