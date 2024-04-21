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
        Camera(double field_of_view = 90.0,
               Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        ~Camera();
    public:
        double aperture_width = 0.16;
        double aperture_height = 0.09;
        double field_of_view;
    public:
        float focal_length() const; // the length in  meter between the center of the aperture and the focal point
    };
}