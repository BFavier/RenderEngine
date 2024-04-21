#include <RenderEngine/graphics/Camera.hpp>
#include <cmath>  // for tan
using namespace RenderEngine;

Camera::Camera(const std::shared_ptr<GPU>& gpu, float _aperture_width, float _aperture_height, float _field_of_view, Referential* _parent, Vector _position, Quaternion _orientation, double _scale) :
    Referential(_parent, _position, _orientation, _scale), aperture_width(_aperture_width), aperture_height(_aperture_height), field_of_view(_field_of_view), _parameters(gpu, sizeof(CameraParameters), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
}

Camera::~Camera()
{
}

float Camera::focal_length() const
{
    if (field_of_view > 0.0 && field_of_view < 180.0)
    {
        return std::max(aperture_width, aperture_height) / 2.0 / tan(field_of_view/2.0 * PI/180.);
    }
    else
    {
        return 0.f;
    }
}
