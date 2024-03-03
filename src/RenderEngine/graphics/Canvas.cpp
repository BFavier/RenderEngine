#include <RenderEngine/render_engine.hpp>
#include <RenderEngine/graphics/Canvas.hpp>
using namespace RenderEngine;

Canvas::Canvas(const std::shared_ptr<GPU>& _gpu, uint32_t width, uint32_t height) :
    gpu(_gpu),
    image(_gpu, width, height, Image::RGBA),
    handles(_gpu, width, height, Image::Format::POINTER, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false),
    depth_buffer(_gpu, width, height, Image::Format::DEPTH, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, false)
{
    allocate_frame_buffer();
    allocate_command_buffer(_draw_command_buffer, std::get<2>(gpu->_graphics_family_queue.value()));
    allocate_command_buffer(_fill_command_buffer, std::get<2>(gpu->_transfer_family_queue.value()));
}


Canvas::Canvas(const Image& _image) :
    gpu(_image.gpu),
    image(_image),
    handles(_image.gpu, _image.width(), _image.height(), Image::Format::POINTER, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false),
    depth_buffer(_image.gpu, _image.width(), _image.height(), Image::Format::DEPTH, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, false)
{
    allocate_frame_buffer();
    allocate_command_buffer(_draw_command_buffer, std::get<2>(gpu->_graphics_family_queue.value()));
    allocate_command_buffer(_fill_command_buffer, std::get<2>(gpu->_transfer_family_queue.value()));
}

Canvas::~Canvas()
{
}


void Canvas::allocate_frame_buffer()
{
    _frame_buffer.reset(new VkFramebuffer, std::bind(&Canvas::_deallocate_frame_buffer, gpu, std::placeholders::_1));
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
    allocInfo.commandPool = std::get<2>(gpu->_graphics_family_queue.value());
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(gpu->_logical_device, &allocInfo, command_buffer.get()) != VK_SUCCESS);
    {
        THROW_ERROR("failed to allocate command buffers!");
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


void Canvas::draw()
{
    // vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
    // vkResetFences(device, 1, &inFlightFence);

    // vkResetCommandBuffer(*_command_buffer, 0); // second argument can be a cominaison of VkCommandBufferResetFlagBits
    // recordCommandBuffer(*_command_buffer, imageIndex);

    // VkSubmitInfo submitInfo{};
    // submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    // VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    // submitInfo.waitSemaphoreCount = 1;
    // submitInfo.pWaitSemaphores = waitSemaphores;
    // submitInfo.pWaitDstStageMask = waitStages;

    // submitInfo.commandBufferCount = 1;
    // submitInfo.pCommandBuffers = _command_buffer.get();

    // VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    // submitInfo.signalSemaphoreCount = 1;
    // submitInfo.pSignalSemaphores = signalSemaphores;

    // if (vkQueueSubmit(std::get<1>(gpu->_graphics_family_queue.value()), 1, &submitInfo, inFlightFence) != VK_SUCCESS)
    // {
    //     throw std::runtime_error("failed to submit draw command buffer!");
    // }

    // VkPresentInfoKHR presentInfo{};
    // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // presentInfo.waitSemaphoreCount = 1;
    // presentInfo.pWaitSemaphores = signalSemaphores;

    // VkSwapchainKHR swapChains[] = {swapChain};
    // presentInfo.swapchainCount = 1;
    // presentInfo.pSwapchains = swapChains;
    // presentInfo.pImageIndices = &imageIndex;

    // vkQueuePresentKHR(presentQueue, &presentInfo);
}