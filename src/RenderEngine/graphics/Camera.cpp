#include <RenderEngine/graphics/Camera.hpp>
#include <cmath>  // for tan
using namespace RenderEngine;

Camera::Camera(float _horizontal_field_of_view, float _near_plane, float _far_plane,
               Vector _position, Quaternion _orientation, double _scale, Referential* _parent) :
    Referential(_position, _orientation, _scale, _parent)
{
    near_plane_distance = _near_plane;
    far_plane_distance = _far_plane;
    horizontal_length = 2.0 * std::tan(_horizontal_field_of_view / 2.0) * _near_plane;
}

Camera::Camera(float _horizontal_span, float _far_plane,
               Vector _position, Quaternion _orientation, double _scale, Referential* _parent) :
    Referential(_position, _orientation, _scale, _parent)
{
    near_plane_distance = 0.;
    far_plane_distance = _far_plane;
    horizontal_length = _horizontal_span;
}

Camera::~Camera()
{
}
