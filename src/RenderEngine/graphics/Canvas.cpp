#include <RenderEngine/render_engine.hpp>
#include <RenderEngine/graphics/Canvas.hpp>
using namespace RenderEngine;

Canvas::Canvas(std::shared_ptr<GPU> _gpu, uint32_t width, uint32_t height) :
    image(_gpu, width, height, Image::RGBA),
    handles(_gpu, width, height, Image::Format::POINTER, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false),
    depth_buffer(_gpu, width, height, Image::Format::DEPTH, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, false)
{
}


Canvas::Canvas(const Image& _image) :
    image(_image),
    handles(_image.gpu, _image.width(), _image.height(), Image::Format::POINTER, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, false),
    depth_buffer(_image.gpu, _image.width(), _image.height(), Image::Format::DEPTH, Image::AntiAliasing::X1, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, false)
{
}

Canvas::~Canvas()
{
}


void Canvas::allocate_frame_buffer()
{
    VkFramebufferCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    info.renderPass = renderPass;
    info.attachmentCount = 1;
    info.pAttachments = attachments;
    info.width = swapChainExtent.width;
    info.height = swapChainExtent.height;
    info.layers = 1;

    if (vkCreateFramebuffer(gpu._logical_device, &info, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void _deallocate_frame_buffer()

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