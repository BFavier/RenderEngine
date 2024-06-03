#include <RenderEngine/graphics/model/Model.hpp>
using namespace RenderEngine;


Model::Model() : Referential()
{
}

Model::Model(const std::shared_ptr<Mesh>& _mesh, const Vector& position, const Quaternion orientation, double scale, Referential* parent) :
    Referential(position, orientation, scale, parent)
{
    mesh = _mesh;
}

Model::~Model()
{
}