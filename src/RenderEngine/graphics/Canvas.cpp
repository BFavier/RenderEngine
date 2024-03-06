#include <RenderEngine/render_engine.hpp>
#include <RenderEngine/graphics/Canvas.hpp>
using namespace RenderEngine;

Canvas::Canvas(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height) :
    gpu(_gpu),
    image(_gpu, width, height, Format::RGBA),
    handles(_gpu, width, height, Format::POINTER, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false),
    depth_buffer(_gpu, width, height, Format::DEPTH, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false)
{
    allocate_frame_buffer();
    allocate_command_buffer(_draw_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    // allocate_command_buffer(_fill_command_buffer, std::get<2>(gpu->_transfer_queue.value()));
    allocate_fence();
    allocate_semaphore();
}


Canvas::Canvas(const std::shared_ptr<GPU>& gpu, uint32_t width, uint32_t height, const std::shared_ptr<VkImage>& vk_image) :
    gpu(gpu),
    image(gpu, width, height, Format::RGBA, vk_image),
    handles(gpu, width, height, Format::POINTER, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false),
    depth_buffer(gpu, width, height, Format::DEPTH, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false)
{
    allocate_frame_buffer();
    allocate_command_buffer(_draw_command_buffer, std::get<2>(gpu->_graphics_queue.value()));
    // allocate_command_buffer(_fill_command_buffer, std::get<2>(gpu->_transfer_queue.value()));
    allocate_fence();
    allocate_semaphore();
}

Canvas::~Canvas()
{
}


void Canvas::allocate_frame_buffer()
{
    _frame_buffer.reset(new VkFramebuffer, [&](VkFramebuffer* frm_buffer) {Canvas::_deallocate_frame_buffer(gpu, frm_buffer);});
    std::vector<VkImageView> attachments = {*image._vk_image_view, *depth_buffer._vk_image_view};
    VkFramebufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = gpu->shader3d->_render_pass;
    info.attachmentCount = attachments.size();
    info.pAttachments = attachments.data();
    info.width = image.width();
    info.height = image.height();
    info.layers = 1;
    if (vkCreateFramebuffer(gpu->_logical_device, &info, nullptr, _frame_buffer.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create framebuffer");
    }
}


void Canvas::allocate_command_buffer(std::shared_ptr<VkCommandBuffer>& command_buffer, VkCommandPool pool)
{
    command_buffer.reset(new VkCommandBuffer, [&](VkCommandBuffer* cmd_buffer) {Canvas::_deallocate_command_buffer(gpu, pool, cmd_buffer);});
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
    _rendered_fence.reset(new VkFence, [&](VkFence* fnc) {Canvas::_deallocate_fence(gpu, fnc);});
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(gpu->_logical_device, &fenceInfo, nullptr, _rendered_fence.get()) != VK_SUCCESS)
    {
        THROW_ERROR("failed to create VkFence");
    }
}


void Canvas::allocate_semaphore()
{
    _rendered_semaphore.reset(new VkSemaphore, [&](VkSemaphore* smp) {Canvas::_deallocate_semaphore(gpu, smp);});
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


void Canvas::_wait_completion()
{
    if (_rendering)
    {
        // reset dependencies
        _dependencies.clear();
        // wait for previous rendering completion, and reset fence to unsignaled status
        vkWaitForFences(gpu->_logical_device, 1, _rendered_fence.get(), VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(gpu->_logical_device, 1, _rendered_fence.get());
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
        // begin render pass
        std::vector<VkClearValue> clear_values;
        for (unsigned int i=0; i<3;i++)
        {
            VkClearValue clear_value = {};
            clear_value.color = {{0.0f, 0.0f, 0.0f, 1.0f}};
            clear_value.depthStencil = {1.0f, 0};
            clear_values.push_back(clear_value);
        }
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = gpu->shader3d->_render_pass;
        renderPassInfo.framebuffer = *_frame_buffer;
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = {image.width(), image.height()};
        renderPassInfo.clearValueCount = clear_values.size();
        renderPassInfo.pClearValues = clear_values.data();
        vkCmdBeginRenderPass(*_draw_command_buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        // bind the successive pipelines
        for (unsigned int i=0;i<gpu->shader3d->_pipelines.size();i++)
        {
            vkCmdBindPipeline(*_draw_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpu->shader3d->_pipelines[i]);
            //vkCmdBindDescriptorSets(*_draw_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, gpu->shader3d->_pipeline_layouts[i], 0, 1, &descriptorSets.scene, 0, nullptr);
        }
        // set viewport and scissor
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(image.width());
        viewport.height = static_cast<float>(image.height());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(*_draw_command_buffer, 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = {image.width(), image.height()};
        vkCmdSetScissor(*_draw_command_buffer, 0, 1, &scissor);
        // reset the rendering flag
        _rendering = false;
    }
}


void Canvas::draw()
{
    _wait_completion();
    vkCmdDraw(*_draw_command_buffer, 3, 1, 0, 0);
}


void Canvas::render()
{
    // nothing new to render
    if (_rendering)
    {
        return;
    }
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
    // set the rendering flag
    _rendering = true;
}

bool Canvas::rendering()
{
    return _rendering;
}