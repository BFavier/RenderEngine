#include <RenderEngine/graphics/Camera.hpp>
#include <cmath>  // for tan
using namespace RenderEngine;

Camera::Camera(double _field_of_view, Vector _position, Quaternion _orientation, double _scale, Referential* _parent) :
    Referential(_position, _orientation, _scale, _parent), field_of_view(_field_of_view)
{
}

Camera::~Camera()
{
}

float Camera::focal_length() const
{
    if (field_of_view > 0.0 && field_of_view < 180.0)
    {
        return static_cast<float>(std::max(aperture_width, aperture_height) / 2.0 / tan(field_of_view/2.0 * PI/180.));
    }
    else
    {
        return 0.0f;
    }
}
