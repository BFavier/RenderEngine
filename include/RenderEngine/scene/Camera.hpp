#pragma once
#include <RenderEngine/scene/Referential.hpp>
#include <RenderEngine/geometry/Matrix.hpp>


namespace RenderEngine
{
    enum ProjectionType {NONE=0, ORTHOGRAPHIC=1, EQUIRECTANGULAR=2, PERSPECTIVE=3};

    // A Camera object defines the geometrical volume, inside of which things are rendered, and with which projection.
    // The Camera class is a base class that can't be constructed directly.
    class Camera : public Referential
    {
    public:
        Camera() = delete;
        ~Camera();
    protected:
        Camera(float sensitivity, float aperture_width, float focal_length, float max_distance, ProjectionType projection_type,
               Vector position, Quaternion orientation, double scale, Referential* parent);
    public:
        float aperture_width; // Width in spatial unit of the projection's plane
        float focal_length; // distance between focal point and projection's plane.
        float max_distance; // distance in spatial unit beyond which things are not rendered.
        float sensitivity; // light sensitivity of the camera.
        ProjectionType projection_type;
    };

    // Builds a perspective camera, which draws everything in the square pyramid between the camera and max_distance plane, projected onto a point positioned at a 'focal_length' distance behind the camera.
    class PerspectiveCamera : public Camera
    {
    public:
        PerspectiveCamera() = delete;
        PerspectiveCamera(float horizontal_field_of_view, float sensitivity=1.0, float focal_length=1.0, float max_distance=1000.0,
                          Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        ~PerspectiveCamera();
    };

    // Builds a orthographic camera, which draws everything lying in a rectangular cuboide of width 'horizontal_length' and depth 'far_plane'.
    class OrthographicCamera : public Camera
    {
    public:
        OrthographicCamera() = delete;
        OrthographicCamera(float aperture_width, float sensitivity=1.0, float max_distance=1000.0,
                           Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        ~OrthographicCamera();
    };

    // Builds a spherical camera, which draws everything at 360° in a sphere of radius 'max_distance'.
    class SphericalCamera : public Camera
    {
    public:
        SphericalCamera() = delete;
        SphericalCamera(float sensitivity=1.0, float max_distance=1000.0,
                        Vector position = {0., 0., 0.}, Quaternion orientation = {}, double scale=1.0, Referential* parent = nullptr);
        ~SphericalCamera();
    };
}
