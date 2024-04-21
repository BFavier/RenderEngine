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
        Camera(const std::shared_ptr<GPU>& gpu, float sensero_width = 0.16, float aperture_height = 0.09, float field_of_view = 90.0,
               Referential* parent = nullptr, Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0);
        ~Camera();
    public:
        float aperture_width = 0.16;
        float aperture_height = 0.09;
        float field_of_view = 45.0;
    public:
        float focal_length() const;
    protected:
        Buffer _parameters;
    };
}