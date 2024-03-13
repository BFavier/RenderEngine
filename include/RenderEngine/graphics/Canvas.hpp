#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/Image.hpp>
#include <RenderEngine/geometry/Matrix.hpp>


namespace RenderEngine
{
    class Mesh;

    class Canvas
    // A canvas is an RGBA image that can be drawn onto.
    {
        friend class Window;
        public:
            Canvas() = delete;
            Canvas(const std::shared_ptr<GPU>& gpu,  uint32_t width, uint32_t height,
                   Image::AntiAliasing sample_count = Image::AntiAliasing::X1, bool texture_compatible = false);
            Canvas(const std::shared_ptr<GPU>& gpu, const std::shared_ptr<VkImage>& vk_image, uint32_t width, uint32_t height,
                   Image::AntiAliasing sample_count = Image::AntiAliasing::X1, bool texture_compatible = false);
            ~Canvas();
        public:
            std::shared_ptr<GPU> gpu = nullptr;
            Image color;
            Image handles;
            Image depth_buffer;
            std::shared_ptr<VkSemaphore> _rendered_semaphore = nullptr;  // Semaphore to order rendering dependencies on GPU
            std::set<VkSemaphore> _dependencies;  // Semaphore of dependencies that must be rendered before this Canvas
        protected:
            // bool _transfering = false; // boolean that is set to true when data is beeing transfered to images
            bool _in_render_pass = false;
            bool _recording = false; // boolean that is set to true when CommandBuffers are beeing recorded for draw instructions
            bool _rendering = false; // boolean that is set to true when CommandBuffers have been sent to GPU for rendering to proceed
            // std::shared_ptr<VkSemaphore> _transfered_semaphore = nullptr;  // Semaphore to order clearing beffore draw on GPU
            std::shared_ptr<VkFence> _rendered_fence = nullptr; // Fence that becomes 'signaled' once rendering ends on GPU
            // std::shared_ptr<VkFence> _transfered_fence = nullptr; // Fence that becomes 'signaled' once transfers has ended on GPU
            std::shared_ptr<VkFramebuffer> _frame_buffer = nullptr;
            std::shared_ptr<VkCommandBuffer> _draw_command_buffer = nullptr;
            // std::shared_ptr<VkCommandBuffer> _transfer_command_buffer = nullptr;
        protected:
            void allocate_frame_buffer();
            void allocate_command_buffer(std::shared_ptr<VkCommandBuffer>& command_buffer, VkCommandPool pool);
            void allocate_fence(std::shared_ptr<VkFence>& fence);
            void allocate_semaphore(std::shared_ptr<VkSemaphore>& semaphore);
            void _initialize_recording();
            void _start_render_pass();
            void _end_render_pass();
        public:
            void clear(unsigned char R, unsigned char G, unsigned char B, unsigned char A); // Clear the color image to the given color. Also clear other images (depth buffer, ...)
            void draw(const Mesh& mesh, const Matrix& mesh_rotation);  // Record objects to draw in the command buffer. Rendering only happens once the 'render' method is called.
            void render();  // Send the command buffers to GPU. Does nothing if the canvas is not in recording state, or already in rendering state.
            void wait_completion();  // blocks on CPU side until the rendering on GPU is complete
            bool is_recording() const;  // returns whether the render function was called already
            bool is_rendering() const;  // returns whether the render function was called already
            static void _deallocate_frame_buffer(const std::shared_ptr<GPU>& gpu, VkFramebuffer* frame_buffer);
            static void _deallocate_command_buffer(const std::shared_ptr<GPU>& gpu, const VkCommandPool& pool, VkCommandBuffer* command_buffer);
            static void _deallocate_fence(const std::shared_ptr<GPU>& gpu, VkFence* fence);
            static void _deallocate_semaphore(const std::shared_ptr<GPU>& gpu, VkSemaphore* semaphore);
    };
}