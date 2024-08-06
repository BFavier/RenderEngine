#pragma once
#include <RenderEngine/scene/Referential.hpp>
#include <RenderEngine/scene/model/Model.hpp>
#include <RenderEngine/graphics/Image.hpp>
#include <RenderEngine/graphics/GPU.hpp>
#include <memory>
#include <map>
#include <string>
#include <deque>

namespace RenderEngine
{
    class Scene : public Referential
    {
    public:
        Scene() = delete;
        Scene(const GPU* gpu);
        Scene(const GPU* gpu, const std::string& file_path);
        ~Scene();
    public:
        const GPU* gpu;
        std::map<const std::string, std::shared_ptr<Image>> textures;
        std::map<const std::string, std::pair<std::shared_ptr<Mesh>, std::vector<Face>>> meshes;
    public:
        void save_to_disk(const std::string& file_path) const;
    protected:
        std::deque<std::shared_ptr<Referential>> _owned_nodes; // the nodes that were created by a Scene constructor, and that should be deleted once not used by anyone anymore.
    protected:
        void _load_nodes_recursively(aiNode *node, const aiScene *scene, Referential* referential);
        Model _load_model(aiNode* node, const aiScene* scene);
    };
}