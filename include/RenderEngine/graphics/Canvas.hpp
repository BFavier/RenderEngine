#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/Image.hpp>


namespace RenderEngine
{
    class Canvas
    // A canvas is an RGBA image that can be drawn onto.
    {
        public:
            Canvas() = delete;
            Canvas(const std::shared_ptr<GPU>& gpu,  uint32_t width, uint32_t height,
                   Image::AntiAliasing sample_count = Image::AntiAliasing::X1, bool texture_compatible = false);
            Canvas(const std::shared_ptr<GPU>& gpu, const std::shared_ptr<VkImage>& vk_image, uint32_t width, uint32_t height,
                   Image::AntiAliasing sample_count = Image::AntiAliasing::X1, bool texture_compatible = false);
            ~Canvas();
        public:
            std::shared_ptr<GPU> gpu = nullptr;
            Image image;
            Image handles;
            Image depth_buffer;
            std::shared_ptr<VkSemaphore> _rendered_semaphore = nullptr;  // Semaphore to order rendering dependencies on GPU
            std::set<VkSemaphore> _dependencies;  // Semaphore of dependencies that must be rendered before this Canvas
        protected:
            bool _rendering = true; // boolean that is set to true when CommandBuffers are sent to GPU for rendering to proceed
            std::shared_ptr<VkFence> _rendered_fence = nullptr; // Fence that becomes 'signaled' once rendering ends on GPU (is initialized signaled)
            std::shared_ptr<VkFramebuffer> _frame_buffer = nullptr;
            std::shared_ptr<VkCommandBuffer> _draw_command_buffer = nullptr;
            // std::shared_ptr<VkCommandBuffer> _fill_command_buffer = nullptr;
        protected:
            void allocate_frame_buffer();
            void allocate_command_buffer(std::shared_ptr<VkCommandBuffer>& command_buffer, VkCommandPool pool);
            void allocate_fence();
            void allocate_semaphore();
        public:
            void draw();  // Record objects to draw in the command buffer. Rendering only happens once the 'render' method is called.
            void render();  // Send the command buffers to GPU
            void wait_completion(); // blocks on CPU side until the rendering on GPU is complete
            bool is_rendering() const;  // returns whether the render function was called already
            static void _deallocate_frame_buffer(const std::shared_ptr<GPU>& gpu, VkFramebuffer* frame_buffer);
            static void _deallocate_command_buffer(const std::shared_ptr<GPU>& gpu, const VkCommandPool& pool, VkCommandBuffer* command_buffer);
            static void _deallocate_fence(const std::shared_ptr<GPU>& gpu, VkFence* fence);
            static void _deallocate_semaphore(const std::shared_ptr<GPU>& gpu, VkSemaphore* semaphore);
    };
}