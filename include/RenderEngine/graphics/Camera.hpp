#pragma once
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/geometry/Referential.hpp>
#include <RenderEngine/geometry/Matrix.hpp>
#include <RenderEngine/graphics/Buffer.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>


namespace RenderEngine
{
    // A Camera is a 2D rectangle
    class Camera : public Referential
    {
    public:
        Camera() = delete;
        // Build from a GPU, camera opening's width and height (in meters), field of view (in ° along the biggest dimension between width and height)
        Camera(const std::shared_ptr<GPU>& gpu, float width = 0.16, float height = 0.09, float field_of_view = 90.0,
               Referential* parent = nullptr, Vector position = {0., 0., 0.}, Quaternion orientation = {});
        ~Camera();
    public:
        float width = 0.16;
        float height = 0.09;
        float field_of_view = 45.0;
    public:
        float focal_length() const;
        // Reads the camera's settings and absolute position, and uploads it's projection parameters to GPU memory
        VkDescriptorBufferInfo get_projection() const;
    protected:
        Buffer _parameters;
    };
}