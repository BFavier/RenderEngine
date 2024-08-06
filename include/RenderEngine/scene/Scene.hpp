#pragma once
#include <RenderEngine/scene/Referential.hpp>
#include <RenderEngine/scene/model/Model.hpp>
#include <RenderEngine/graphics/Image.hpp>
#include <memory>
#include <map>
#include <string>

namespace RenderEngine
{
    class Scene : public Referential
    {
    public:
        Scene();
        Scene(const std::string& file_path);
        ~Scene();
    public:
        std::map<const std::string, std::shared_ptr<Image>> textures;
        std::map<const std::string, std::shared_ptr<Mesh>> meshes;
    public:
        void save_to_disk(const std::string& file_path) const;
    };
}