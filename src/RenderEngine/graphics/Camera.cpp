#include <RenderEngine/graphics/Camera.hpp>
#include <cmath>  // for tan
using namespace RenderEngine;


Camera::Camera(float _horizontal_length, float _near_plane_distance, float _far_plane_distance, float _sensitivity,
               Vector position, Quaternion orientation, double scale, Referential* parent) :
    Referential(position, orientation, scale, parent)
{
    horizontal_length = _horizontal_length;
    near_plane_distance = _near_plane_distance;
    far_plane_distance = _far_plane_distance;
    sensitivity = _sensitivity;
}

Camera::~Camera()
{
}

PerspectiveCamera::PerspectiveCamera(float horizontal_field_of_view, float near_plane_distance, float far_plane_distance, float _sensitivity,
                                     Vector position, Quaternion orientation, double scale, Referential* _parent) :
    Camera(2.0 * std::tan(horizontal_field_of_view / 2.0) * near_plane_distance, near_plane_distance, far_plane_distance, _sensitivity,
           position, orientation, scale, _parent)
{
}

PerspectiveCamera::~PerspectiveCamera()
{
}

OrthographicCamera::OrthographicCamera(float horizontal_length, float far_plane_distance, float _sensitivity,
               Vector position, Quaternion orientation, double scale, Referential* parent) :
    Camera(horizontal_length, 0.0, far_plane_distance, _sensitivity,
           position, orientation, scale, parent)
{
}

OrthographicCamera::~OrthographicCamera()
{
}