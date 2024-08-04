#include <RenderEngine/graphics/model/Material.hpp>
using namespace RenderEngine;

Material::Material()
{
}

Material::Material(float _metalness, float _roughness, float _ambient_occlusion)
{
    metalness = _metalness;
    roughness = _roughness;
    ambient_occlusion = _ambient_occlusion;
}

Material::~Material()
{
}

vec3 Material::to_vec() const
{
    return vec3({metalness, roughness, ambient_occlusion});
}