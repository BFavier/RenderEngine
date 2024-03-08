#include <RenderEngine/graphics/model/Mesh.hpp>
using namespace RenderEngine;

Mesh::Mesh(const std::shared_ptr<GPU>& _gpu, const std::vector<Face>& faces) : Buffer(gpu, faces.size() * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
    std::vector<Vertex> vertices(faces.size());
    for (unsigned int i=0; i<faces.size();i++)
    {
        const Face& face = faces[i];
        vertices[i] = Vertex({{}, {}, {}});
    }
    upload(vertices.data(), vertices.size() * sizeof(Vertex), 0);
}

Mesh::~Mesh()
{
}
