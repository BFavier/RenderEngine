#pragma once
#include <RenderEngine/graphics/model/Mesh.hpp>
#include <RenderEngine/geometry/Referential.hpp>

namespace RenderEngine
{
    class Model : public Referential
    {
    public:
        Model();
        Model(const std::shared_ptr<Mesh>& _mesh, const Vector& position={}, const Quaternion orientation={}, double scale=1.0, Referential* parent=nullptr);
        ~Model();
    public:
        std::shared_ptr<Mesh> mesh = nullptr;
    };
}
