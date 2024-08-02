#pragma once
#include <RenderEngine/utilities/External.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <RenderEngine/graphics/shaders/Shader.hpp>
#include <RenderEngine/graphics/Image.hpp>
#include <RenderEngine/geometry/Quaternion.hpp>
#include <RenderEngine/geometry/Vector.hpp>
#include <RenderEngine/geometry/Matrix.hpp>
#include <RenderEngine/graphics/Camera.hpp>
#include <RenderEngine/graphics/Light.hpp>
#include <RenderEngine/graphics/Color.hpp>
#include <memory>


namespace RenderEngine
{
    class Mesh;

    class Canvas
    // A canvas is an RGBA image that can be drawn onto.
    {
        friend class SwapChain;
        friend class Window;
        public:
            Canvas() = delete;
            Canvas(const Canvas& other) = delete;
            Canvas& operator=(const Canvas& other) = delete;
        public:
            Canvas(const std::shared_ptr<GPU>& gpu,  uint32_t width, uint32_t height,
                   bool mip_maped = false, AntiAliasing sample_count = AntiAliasing::X1);
            Canvas(const std::shared_ptr<GPU>& gpu, const VkImage& vk_image, uint32_t width, uint32_t height,
                   AntiAliasing sample_count = AntiAliasing::X1);
            ~Canvas();
        public:
            std::shared_ptr<GPU> gpu;
            const std::map<std::string, std::shared_ptr<Image>> images;
            const uint32_t width;
            const uint32_t height;
        protected:
            bool _recording = false; // boolean that is set to true when commands are beeing recorded on cpu
            bool _rendering = false; // boolean that is set to true when commands have been sent to GPU for rendering, and wait_completion has not been called yet.
            const Shader* _current_shader = nullptr; // Shader currently in use
            std::set<const Canvas*> _dependencies;  // Canvas dependencies that must be rendered before this Canvas
            std::set<VkSemaphore> _wait_semaphores;  // Other VkSemaphore that must be waited befoire starting to render (SwapChain image acquisition, ...)
            VkSemaphore _vk_rendered_semaphore = VK_NULL_HANDLE;  // Semaphore to order rendering Canvas dependencies on GPU
            VkFence _vk_fence = VK_NULL_HANDLE; // Fence that becomes 'signaled' on CPU once rendering ends on GPU
            std::map<const Shader*, VkFramebuffer> _frame_buffers;
            VkCommandBuffer _vk_command_buffer = VK_NULL_HANDLE;
            VkImageLayout _final_layout = VK_IMAGE_LAYOUT_UNDEFINED;  // The final layout the color image is converted to at the end of the command buffer
        public:
            void clear(Color color);  // Clear the color image to the given color. Also clear other images (depth buffer, ...)
            void draw(const Camera& camera, const std::shared_ptr<Mesh>& mesh, const std::tuple<Vector, Quaternion, double>& mesh_coordinates_in_camera, bool cull_back_faces);  // Record objects to draw in the command buffer. Rendering only starts once the 'render' method is called.
            void light(const Camera& camera, const Light& light, const std::tuple<Vector, Quaternion, double>& light_coordinates_in_camera);  // light the scene
            void render();  // Send the command buffers to GPU. Does nothing if the canvas is not in recording state, or already in rendering state. This command is asynchrone, and completion is garanteed only once 'wait_completion' is called.
            void wait_completion();  // blocks on CPU side until the rendering on GPU is complete
            bool is_recording() const;  // returns whether the render function was called already
            bool is_rendering() const;  // returns whether the render function was called already
        protected:
            VkFramebuffer _allocate_frame_buffer(const Shader* shader);
            void _allocate_command_buffer(VkCommandBuffer& command_buffer, VkCommandPool pool);
            void _allocate_fence(VkFence& fence);
            void _allocate_semaphore(VkSemaphore& semaphore);
            void _record_commands();
            void _bind_shader(const Shader* shader);
            void _bind_descriptor_set(const Shader* shader,
                int descriptor_set_index,
                const std::map<std::string, std::shared_ptr<Image>>& images_pool,
                const std::map<std::string, std::shared_ptr<Buffer>>& buffers_pool);
            void _command_barrier(const std::map<std::string, VkImageLayout>& new_image_layouts); // set up a command barrier that ensures next commands will be executed after previous commands are finished, and transition the layout of the given images
    };
}