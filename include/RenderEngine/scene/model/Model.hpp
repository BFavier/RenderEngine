#pragma once
#include <RenderEngine/scene/model/Mesh.hpp>
#include <RenderEngine/scene/Referential.hpp>

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
        bool two_sided = false;
        std::optional<std::shared_ptr<Image>> texture;
        std::optional<std::shared_ptr<Image>> normal_maping;
        std::optional<std::shared_ptr<Image>> ambient_occlusion_maping;
    };
}
