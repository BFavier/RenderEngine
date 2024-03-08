#include <RenderEngine/graphics/model/Mesh.hpp>
using namespace RenderEngine;

Mesh::Mesh(const std::shared_ptr<GPU>& _gpu, const std::vector<Face>& faces, std::array<float, 4> color) : gpu(_gpu)
{
    std::vector<Vertex> vertices(faces.size());
    for (unsigned int i=0; i<faces.size();i++)
    {
        const Face& face = faces[i];
        vertices[i] = Vertex({{}, {}, {}});
    }
}

Mesh::~Mesh()
{
}
