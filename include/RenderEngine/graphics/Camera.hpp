#pragma once
#include <RenderEngine/geometry/Referential.hpp>
#include <RenderEngine/geometry/Matrix.hpp>
#include <RenderEngine/graphics/Buffer.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>


namespace RenderEngine
{
    // A Camera object defines the geometrical volume, inside of which things are rendered, and with which projection.
    class Camera : public Referential
    {
    public:
        Camera() = delete;
        // Builds a perspective camera, which projects everything in the square pyramid between near and far plane onto the near plane
        Camera(float horizontal_field_of_view, float near_plane_distance, float far_plane_distance,
               Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        // Builds a orthogonal-projection camera, which draws everything lying in a rectangular cuboide of width 'horizontal_length' and depth 'far_plane'.
        Camera(float horizontal_length, float far_plane_distance,
               Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        ~Camera();
    public:
        float near_plane_distance;  // distance in spatial unit of the near plane (before which things are not rendered)
        float far_plane_distance; // distance in spatial unit of the far plane (beyond which things are not rendered)
        float horizontal_length; // Width in spatial unit of the projection's plane (near plane)
    };
}