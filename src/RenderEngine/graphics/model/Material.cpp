#include <RenderEngine/graphics/model/Material.hpp>
using namespace RenderEngine;

Material::Material()
{
}

Material::Material(double _metalness, double _roughness, double _luminance)
{
    metalness = _metalness;
    roughness = _roughness;
    luminance = _luminance;
}

Material::~Material()
{
}

vec3 Material::to_vec() const
{
    return vec3({static_cast<float>(metalness), static_cast<float>(roughness), static_cast<float>(luminance)});
}