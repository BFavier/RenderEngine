#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/Image.hpp>
#include <RenderEngine/geometry/Quaternion.hpp>
#include <RenderEngine/geometry/Vector.hpp>
#include <RenderEngine/geometry/Matrix.hpp>
#include <RenderEngine/graphics/Camera.hpp>


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
            bool _recording = false; // boolean that is set to true when commands are beeing recorded on cpu
            bool _rendering = false; // boolean that is set to true when commands have been sent to GPU for rendering, and wait_completion has not been called yet.
            bool _recording_render_pass = false; // boolean whether the command buffer is recording a render pass. Some commands can be issued only when recording a render pass (draw calls, ...). Others must only be issued outside of a render pass (clearing, copies, layout transitions ...)
            std::shared_ptr<VkFence> _rendered_fence = nullptr; // Fence that becomes 'signaled' once rendering ends on GPU
            std::shared_ptr<VkFramebuffer> _frame_buffer = nullptr;
            std::shared_ptr<VkCommandBuffer> _command_buffer = nullptr;
            std::shared_ptr<Buffer> _camera_view = nullptr;
        protected:
            void _allocate_frame_buffer();
            void _allocate_command_buffer(std::shared_ptr<VkCommandBuffer>& command_buffer, VkCommandPool pool);
            void _allocate_fence(std::shared_ptr<VkFence>& fence);
            void _allocate_semaphore(std::shared_ptr<VkSemaphore>& semaphore);
            void _allocate_camera_view(std::shared_ptr<Buffer>& camera_view);
            void _initialize_recording();
            void _start_render_pass_recording();
            void _end_render_pass_recording();
        public:
            // Clear the color image to the given color. Also clear other images (depth buffer, ...)
            void clear(unsigned char R, unsigned char G, unsigned char B, unsigned char A);
            // Set the view for the 3D rendering to the current state of the given camera
            void set_view(const Camera& camera);
            // Record objects to draw in the command buffer. Rendering only starts once the 'render' method is called.
            void draw(const Mesh& mesh, const std::tuple<Vector, Quaternion, double>& coordinates_in_camera, bool cull_back_faces);
            // Send the command buffers to GPU. Does nothing if the canvas is not in recording state, or already in rendering state. This command is asynchrone, and completion is garanteed only once 'wait_completion' is called.
            void render();
            // blocks on CPU side until the rendering on GPU is complete
            void wait_completion();
            bool is_recording() const;  // returns whether the render function was called already
            bool is_rendering() const;  // returns whether the render function was called already
            static void _deallocate_frame_buffer(const std::shared_ptr<GPU>& gpu, VkFramebuffer* frame_buffer);
            static void _deallocate_command_buffer(const std::shared_ptr<GPU>& gpu, const VkCommandPool& pool, VkCommandBuffer* command_buffer);
            static void _deallocate_fence(const std::shared_ptr<GPU>& gpu, VkFence* fence);
            static void _deallocate_semaphore(const std::shared_ptr<GPU>& gpu, VkSemaphore* semaphore);
    };
}