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
}


Canvas::Canvas(const Image& _image) :
    gpu(_image.gpu),
    image(_image),
    handles(_image.gpu, _image.width(), _image.height(), Image::Format::POINTER, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false),
    depth_buffer(_image.gpu, _image.width(), _image.height(), Image::Format::DEPTH, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, false)
{
    allocate_frame_buffer();
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
        throw std::runtime_error("failed to create framebuffer!");
    }

}

void _deallocate_frame_buffer(const std::shared_ptr<GPU>& gpu, VkFramebuffer* frame_buffer)
{
    vkDestroyFramebuffer(gpu->_logical_device, *frame_buffer, nullptr);
}

// void Canvas::draw()
// {
//     vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);
//     vkResetFences(device, 1, &inFlightFence);

//     uint32_t imageIndex;
//     vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

//     vkResetCommandBuffer(commandBuffer, /*VkCommandBufferResetFlagBits*/ 0);
//     recordCommandBuffer(commandBuffer, imageIndex);

//     VkSubmitInfo submitInfo{};
//     submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

//     VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
//     VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
//     submitInfo.waitSemaphoreCount = 1;
//     submitInfo.pWaitSemaphores = waitSemaphores;
//     submitInfo.pWaitDstStageMask = waitStages;

//     submitInfo.commandBufferCount = 1;
//     submitInfo.pCommandBuffers = &commandBuffer;

//     VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
//     submitInfo.signalSemaphoreCount = 1;
//     submitInfo.pSignalSemaphores = signalSemaphores;

//     if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
//         throw std::runtime_error("failed to submit draw command buffer!");
//     }

//     VkPresentInfoKHR presentInfo{};
//     presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

//     presentInfo.waitSemaphoreCount = 1;
//     presentInfo.pWaitSemaphores = signalSemaphores;

//     VkSwapchainKHR swapChains[] = {swapChain};
//     presentInfo.swapchainCount = 1;
//     presentInfo.pSwapchains = swapChains;

//     presentInfo.pImageIndices = &imageIndex;

//     vkQueuePresentKHR(presentQueue, &presentInfo);
// }