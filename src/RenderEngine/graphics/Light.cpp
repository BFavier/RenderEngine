#include <RenderEngine/graphics/Light.hpp>
using namespace RenderEngine;

Light::Light(Color _color, double _luminance) : color(_color), luminance(_luminance)
{
    
}

Light::~Light()
{
}


PointLight::PointLight(Color _color, double _luminance,
                       float _horizontal_field_of_view, float _near_plane, float _far_plane,
                       Vector _position, Quaternion _orientation, double _scale, Referential* _parent) :
    Light(_color, _luminance), PerspectiveCamera(_horizontal_field_of_view, _near_plane, _far_plane, _position, _orientation, _scale, _parent)
{
}

PointLight::~PointLight()
{
}

DirectionalLight::DirectionalLight(Color _color, double _luminance,
                                   float horizontal_length, float far_plane_distance,
                                   Vector _position = {0., 0., 0.}, Quaternion _orientation = {}, double _scale=1.0, Referential* _parent = nullptr) :
    Light(_color, _luminance), OrthographicCamera(horizontal_length, far_plane_distance, _position, _orientation, _scale, _parent)
{
}

DirectionalLight::~DirectionalLight()
{
}