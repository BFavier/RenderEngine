#pragma once
#include <RenderEngine/geometry/Referential.hpp>
#include <RenderEngine/geometry/Matrix.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>


namespace RenderEngine
{
    // A Camera object defines the geometrical volume, inside of which things are rendered, and with which projection.
    // The Camera class is a base class that can't be constructed directly.
    class Camera : public Referential
    {
    public:
        Camera() = delete;
        ~Camera();
    protected:
        Camera(float horizontal_length, float near_plane_distance, float far_plane_distance, float sensitivity,
               Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
    public:
        float horizontal_length; // Width in spatial unit of the projection's plane (near plane)
        float near_plane_distance;  // distance in spatial unit of the near plane (before which things are not rendered). If this is 0, use orthographic projection.
        float far_plane_distance; // distance in spatial unit of the far plane (beyond which things are not rendered)
        float sensitivity; // light sensitivity of the camera
    };

    // Builds a perspective camera, which projects everything in the square pyramid between near and far plane onto the near plane
    class PerspectiveCamera : public Camera
    {
    public:
        PerspectiveCamera() = delete;
        PerspectiveCamera(float horizontal_field_of_view, float near_plane_distance, float far_plane_distance, float sensitivity,
                          Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        ~PerspectiveCamera();
    };

    // Builds a orthographic camera, which draws everything lying in a rectangular cuboide of width 'horizontal_length' and depth 'far_plane'.
    class OrthographicCamera : public Camera
    {
    public:
        OrthographicCamera() = delete;
        OrthographicCamera(float horizontal_length, float far_plane_distance, float sensitivity,
                           Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        ~OrthographicCamera();
    };
}