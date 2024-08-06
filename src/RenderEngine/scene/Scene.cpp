#include <RenderEngine/scene/Scene.hpp>
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/geometry/Matrix.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
using namespace RenderEngine;


Scene::Scene(const GPU* gpu, const std::string& file_path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file_path, aiProcess_Triangulate);
    if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr)
    {
        THROW_ERROR("Failed to load scene from file '"+file_path+"' : "+importer.GetErrorString());
    }
    _load_nodes_recursively(scene->mRootNode, scene);
}

Scene::~Scene()
{
}

void Scene::_load_nodes_recursively(aiNode *node, const aiScene *scene, Referential* referential)
{
    aiMatrix4x4 mat = node->mTransformation;
    Vector position(mat.a4, mat.b4, mat.c4);
    //Matrix rotation({{mat.a1, mat.a1, mat.a2}, {mat.b1, mat.b2, mat.b3}, {mat.c1, mat.c2, mat.c3}});
    if (node->mNumMeshes > 0)
    {
        _owned_nodes.emplace_back(new Referential());
        ...
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        _load_nodes_recursively(node->mChildren[i], scene);
    }
}


Model Scene::_load_model(aiNode* node, const aiScene* scene)
{
    bool two_sided = false;
    std::vector<Face> faces;
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        std::vector<Vector> positions(mesh->mNumVertices);
        std::vector<Vector> normals(mesh->mNumVertices);
        std::vector<UV> uvs(mesh->mNumVertices, UV(0., 0.));
        // read material
        // std::optional<std::string> texture;
        // std::optional<std::string> normal_map;
        // std::optional<std::string> ambient_occlusion_map;
        Color color(0., 0., 0., 1.0);
        Material material(0., 0., 1.);
        aiMaterial* ai_mtl = scene->mMaterials[mesh->mMaterialIndex];
        aiColor4D ai_clr;
        float ai_float;
        int ai_int;
        {
            // two sided
            if (aiGetMaterialInteger(ai_mtl, AI_MATKEY_TWOSIDED, &ai_int) == AI_SUCCESS)
            {
                two_sided = (ai_int != 0);
            }
            // color
            if (aiGetMaterialColor(ai_mtl, AI_MATKEY_COLOR_DIFFUSE, &ai_clr) == AI_SUCCESS)
            {
                if (ai_clr.r != 0. || ai_clr.g != 0. || ai_clr.b != 0.)
                {
                    color = Color(ai_clr.r, ai_clr.g, ai_clr.b, color.a);
                }
            }
            if (aiGetMaterialColor(ai_mtl, AI_MATKEY_COLOR_AMBIENT, &ai_clr) == AI_SUCCESS)
            {
                if (ai_clr.r != 0. || ai_clr.g != 0. || ai_clr.b != 0.)
                {
                    color = Color(ai_clr.r, ai_clr.g, ai_clr.b, color.a);
                }
            }
            if (aiGetMaterialColor(ai_mtl, AI_MATKEY_BASE_COLOR, &ai_clr) == AI_SUCCESS)
            {
                if (ai_clr.r != 0. || ai_clr.g != 0. || ai_clr.b != 0.)
                {
                    color = Color(ai_clr.r, ai_clr.g, ai_clr.b, color.a);
                }
            }
            // metalness
            if (aiGetMaterialFloat(ai_mtl, AI_MATKEY_METALLIC_FACTOR, &ai_float) == AI_SUCCESS)
            {
                material.metalness = ai_float;
            }
            // roughness
            if (aiGetMaterialFloat(ai_mtl, AI_MATKEY_SHININESS, &ai_float) == AI_SUCCESS)
            {
                material.roughness = 1.0 - ai_float;
            }
            if (aiGetMaterialFloat(ai_mtl, AI_MATKEY_ROUGHNESS_FACTOR, &ai_float) == AI_SUCCESS)
            {
                material.roughness = ai_float;
            }
        }
        // loop on vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            const aiVector3D& vertex = mesh->mVertices[i];
            positions[i] = Vector(vertex.x, vertex.y, vertex.z);
            if (mesh->HasNormals())
            {
                const aiVector3D& normal = mesh->mNormals[i];
                normals[i] = Vector(normal.x, normal.y, normal.z);
            }
            // texture coordinates
            if(mesh->mTextureCoords[0] != nullptr) // does the mesh contain texture coordinates?
            {
                const aiVector3D& uv = mesh->mTextureCoords[0][i];
                uvs[i] = UV(uv.x, 1.0 - uv.y); // Assimp's uv coordinates as a (0, 0) origin on bottom left of image instead of top left.
            }
        }
        // loop on mesh faces
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            if (face.mNumIndices != 3)
            {
                THROW_ERROR("Expected triangular faces but found face with number of vertices = "+std::to_string(face.mNumIndices));
            }
            unsigned int a = face.mIndices[0];
            unsigned int b = face.mIndices[1];
            unsigned int c = face.mIndices[2];
            faces.push_back(Face({positions[a], positions[b], positions[c]}, {uvs[a], uvs[b], uvs[c]}, color, material));
            if (mesh->HasNormals())
            {
                faces.back().normals = {normals[a], normals[b], normals[c]};
            }
        }
    }

    return std::make_pair(std::make_shared<Mesh>(gpu, faces), faces);
}
