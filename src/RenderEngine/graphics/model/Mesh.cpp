#include <RenderEngine/graphics/model/Mesh.hpp>
using namespace RenderEngine;

Mesh::Mesh(const std::shared_ptr<GPU>& _gpu, const std::vector<Face>& faces)
{
    _buffer.reset(new Buffer(_gpu, faces.size() * 3 * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
    _offset = 0;
    _bytes_size = faces.size() * sizeof(Vertex) * 3;
    upload(faces);
}

Mesh::~Mesh()
{
}

void Mesh::upload(const std::vector<Face>& faces)
{
    std::vector<Vertex> vertices(faces.size() * 3);
    for (size_t i=0; i<faces.size();i++)
    {
        const Face& face = faces[i];
        vertices[i*3] = {face.points[0].to_vec3(), face.normal.to_vec3(), face.color};
        vertices[i*3+1] = {face.points[1].to_vec3(), face.normal.to_vec3(), face.color};
        vertices[i*3+2] = {face.points[2].to_vec3(), face.normal.to_vec3(), face.color};
    }
    _buffer->upload(reinterpret_cast<uint8_t*>(vertices.data()), faces.size()*sizeof(Vertex)*3, _offset);
}

std::size_t Mesh::bytes_size() const
{
    return _bytes_size;
}
