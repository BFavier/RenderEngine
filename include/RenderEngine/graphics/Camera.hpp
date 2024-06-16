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
        Camera(float field_of_view = 45.0,
               Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        ~Camera();
    public:
        float near_plane = 1.0E-1;
        float far_plane = 1.0E3;
        float field_of_view;
    public:
        float focal_length() const; // the length in  meter between the center of the aperture and the focal point
    };
}