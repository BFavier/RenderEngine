#include <RenderEngine/scene/Camera.hpp>
#include <cmath>  // for tan
using namespace RenderEngine;


Camera::Camera(float _sensitivity, float _aperture_width, float _focal_length, float _max_distance, ProjectionType _projection_type,
               Vector position, Quaternion orientation, double scale, Referential* parent) :
    Referential(position, orientation, scale, parent)
{
    sensitivity = _sensitivity;
    aperture_width = _aperture_width;
    focal_length = _focal_length;
    max_distance = _max_distance;
    projection_type = _projection_type;
}

Camera::~Camera()
{
}

PerspectiveCamera::PerspectiveCamera(float horizontal_field_of_view, float sensitivity, float focal_length, float max_distance,
                                     Vector position, Quaternion orientation, double scale, Referential* parent) :
    Camera(sensitivity,
           2.0 * std::tan(horizontal_field_of_view / 2.0) * focal_length,
           focal_length,
           max_distance,
           ProjectionType::PERSPECTIVE,
           position, orientation, scale, parent)
{
}

PerspectiveCamera::~PerspectiveCamera()
{
}

OrthographicCamera::OrthographicCamera(float aperture_width, float sensitivity, float max_distance,
                                       Vector position, Quaternion orientation, double scale, Referential* parent) :
    Camera(sensitivity,
           aperture_width,
           0.,
           max_distance,
           ProjectionType::ORTHOGRAPHIC,
           position, orientation, scale, parent)
{
}

OrthographicCamera::~OrthographicCamera()
{
}

SphericalCamera::SphericalCamera(float sensitivity, float max_distance,
                                 Vector position, Quaternion orientation, double scale, Referential* parent) :
    Camera(sensitivity,
           0.,
           0.,
           max_distance,
           ProjectionType::EQUIRECTANGULAR,
           position, orientation, scale, parent)
{
}

SphericalCamera::~SphericalCamera()
{
}
