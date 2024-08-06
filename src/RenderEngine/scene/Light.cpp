#include <RenderEngine/scene/Light.hpp>
using namespace RenderEngine;

Light::Light(Color _color, float _intensity,
             float aperture_width, float focal_length, float max_distance, ProjectionType projection_type,
             Vector position, Quaternion orientation, double scale, Referential* parent) :
    Camera(0., aperture_width, focal_length, max_distance, projection_type,
           position, orientation, scale, parent)
{
    color = _color;
    intensity = _intensity;
}

Light::~Light()
{
}

AmbientLight::AmbientLight(Color color, float intensity, Referential* parent) :
    Light(color, intensity, 0., 0., 0., ProjectionType::NONE, Vector(), Quaternion(), 1.0, parent)
{

}

AmbientLight::~AmbientLight()
{
}

DirectionalLight::DirectionalLight(Color color, float intensity, float aperture_width, float max_distance,
                                   Vector position, Quaternion orientation, double scale, Referential* parent) :
    Light(color, intensity, aperture_width, 0., max_distance, ProjectionType::ORTHOGRAPHIC, position, orientation, scale, parent)
{
}

DirectionalLight::~DirectionalLight()
{
}

PointLight::PointLight(Color color, float intensity, float constant_intensity_radius, float max_shadow_distance,
                       Vector position, Quaternion orientation, double scale, Referential* parent) :
    Light(color, intensity, 0., constant_intensity_radius, max_shadow_distance, ProjectionType::EQUIRECTANGULAR, position, orientation, scale, parent)
{
}

PointLight::~PointLight()
{
}
