#include <RenderEngine/graphics/model/Mesh.hpp>
using namespace RenderEngine;

Mesh::Mesh(const GPU* _gpu, const std::vector<Face>& faces)
{
    _buffer.reset(new Buffer(_gpu, faces.size() * 3 * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
    _offset = 0;
    _bytes_size = faces.size() * sizeof(Vertex) * 3;
    upload(faces);
}

Mesh::Mesh(const std::shared_ptr<Buffer>& buffer, std::size_t offset, const std::vector<Face>& faces)
{
    _buffer = buffer;
    _offset = offset;
    _bytes_size = faces.size() * sizeof(Vertex) * 3;
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
        vertices[i*3] = {face.points[0].to_vec3(), face.normals[0].to_vec3(), face.color.to_vec4(), face.UVs[0].to_vec(), face.material.to_vec()};
        vertices[i*3+1] = {face.points[1].to_vec3(), face.normals[1].to_vec3(), face.color.to_vec4(), face.UVs[1].to_vec(), face.material.to_vec()};
        vertices[i*3+2] = {face.points[2].to_vec3(), face.normals[2].to_vec3(), face.color.to_vec4(), face.UVs[2].to_vec(), face.material.to_vec()};
    }
    _buffer->upload(vertices.data(), faces.size()*sizeof(Vertex)*3, _offset);
}

std::size_t Mesh::bytes_size() const
{
    return _bytes_size;
}

std::vector<std::shared_ptr<Mesh>> Mesh::bulk_allocate_meshes(const GPU* gpu, const std::vector<std::vector<Face>>& faces)
{
    std::size_t bytes_size = 0;
    std::vector<std::size_t> offsets;
    for (const std::vector<Face>& mesh : faces)
    {
        offsets.push_back(bytes_size);
        bytes_size += mesh.size() * sizeof(Vertex) * 3;
    }
    std::shared_ptr<Buffer> buffer(new Buffer(gpu, bytes_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));
    std::vector<std::shared_ptr<Mesh>> meshes;
    for (std::size_t i=0; i<faces.size(); i++)
    {
        meshes.emplace_back(new Mesh(buffer, offsets[i], faces[i]));
        meshes[i]->upload(faces[i]);
    }
    return meshes;
}


std::map<std::string, std::shared_ptr<Mesh>> Mesh::bulk_allocate_meshes(const GPU* gpu, const std::map<std::string, std::vector<Face>>& faces)
{
    std::vector<std::string> names;
    std::vector<std::vector<Face>> faces_vector;
    for (const std::pair<std::string, std::vector<Face>>& key_values : faces)
    {
        names.push_back(key_values.first);
        faces_vector.push_back(key_values.second);
    }
    std::vector<std::shared_ptr<Mesh>> meshes = Mesh::bulk_allocate_meshes(gpu, faces_vector);
    std::map<std::string, std::shared_ptr<Mesh>> meshes_map;
    for (std::size_t i=0; i<meshes.size(); i++)
    {
        meshes_map[names[i]] = meshes[i];
    }
    return meshes_map;
}