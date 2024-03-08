#include <RenderEngine/graphics/model/Mesh.hpp>
using namespace RenderEngine;

Mesh::Mesh(const std::shared_ptr<GPU>& _gpu, const std::vector<Face>& faces) : Buffer(_gpu, faces.size() * 3 * sizeof(Vertex), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
{
    std::vector<Vertex> vertices(faces.size() * 3);
    for (size_t i=0; i<faces.size();i++)
    {
        const Face& face = faces[i];
        vertices[i*3] = {face.points[0], face.normal, face.color};
        vertices[i*3+1] = {face.points[1], face.normal, face.color};
        vertices[i*3+2] = {face.points[2], face.normal, face.color};
    }
    upload(vertices.data(), vertices.size() * sizeof(Vertex), 0);
}

Mesh::~Mesh()
{
}


Mesh Mesh::cube(const std::shared_ptr<GPU>& gpu, float r)
{
    std::array<vec3, 3> points = {{{0., 0., 0.}, {r, r, 0.}, {0., r, 0.}}};
    std::vector<Face> faces = {Face({{{0., 0., 0.}, {r, r, 0.}, {0., r, 0.}}}, {1., 0., 0., 1.}),
                               Face({{{0., 0., 0.}, {r, 0., 0.}, {r, r, 0.}}}, {1., 0., 0., 1.}),
                               Face({{{0., 0., r}, {0., r, r}, {r, r, r}}}, {1., 0., 0., 1.}),
                               Face({{{0., 0., r}, {r, r, r}, {r, 0., r}}}, {1., 0., 0., 1.}),

                               Face({{{0., 0., 0.}, {r, 0., r}, {r, 0., 0.}}}, {0., 0., 1., 1.}),
                               Face({{{0., 0., 0.}, {0., 0., r}, {r, 0., r}}}, {0., 0., 1., 1.}),
                               Face({{{0., r, 0.}, {r, r, 0.}, {r, r, r}}}, {0., 0., 1., 1.}),
                               Face({{{0., r, 0.}, {r, r, r}, {0., r, r}}}, {0., 0., 1., 1.}),
                               
                               Face({{{0., 0., 0.}, {0., r, r}, {0., 0., r}}}, {0., 1., 0., 1.}),
                               Face({{{0., 0., 0.}, {0., r, 0.}, {0., r, r}}}, {0., 1., 0., 1.}),
                               Face({{{r, 0., r}, {r, 0., r}, {r, r, r}}}, {0., 1., 0., 1.}),
                               Face({{{r, 0., 0.}, {r, r, r}, {r, r, 0.}}}, {0., 1., 0., 1.})};
    Mesh mesh(gpu, faces);
    return mesh;
}