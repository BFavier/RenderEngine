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

std::vector<Face> Mesh::cube(double length)
{
    double l = length/2;
    std::vector<Face> faces = {Face({{{-l, -l, -l}, {-l, l, -l}, {l, l, -l}}}, {1., 0., 0., 1.}),
                               Face({{{-l, -l, -l}, {l, l, -l}, {l, -l, -l}}}, {1., 0., 0., 1.}),
                               Face({{{-l, -l, l}, {l, l, l}, {-l, l, l}}}, {1., 1., 0., 1.}),
                               Face({{{-l, -l, l}, {l, -l, l}, {l, l, l}}}, {1., 1., 0., 1.}),

                               Face({{{-l, -l, -l}, {l, -l, -l}, {l, -l, l}}}, {1., 0., 1., 1.}),
                               Face({{{-l, -l, -l}, {l, -l, l}, {-l, -l, l}}}, {1., 0., 1., 1.}),
                               Face({{{-l, l, -l}, {l, l, l}, {l, l, -l}}}, {0., 0., 1., 1.}),
                               Face({{{-l, l, -l}, {-l, l, l}, {l, l, l}}}, {0., 0., 1., 1.}),
                               
                               Face({{{-l, -l, -l}, {-l, -l, l}, {-l, l, l}}}, {0., 1., 1., 1.}),
                               Face({{{-l, -l, -l}, {-l, l, l}, {-l, l, -l}}}, {0., 1., 1., 1.}),
                               Face({{{l, -l, -l}, {l, l, l}, {l, -l, l}}}, {0., 1., 0., 1.}),
                               Face({{{l, -l, -l}, {l, l, -l}, {l, l, l}}}, {0., 1., 0., 1.})};
    return faces;
}

std::vector<Face> Mesh::sphere(double radius, uint32_t divides)
{
    std::vector<Face> faces;
    // creating the icosahedron coordinates
    const double phi = (1.0+std::sqrt(5))/2.0;
    const double phi_square = std::sqrt(1 + std::pow(phi, 2));
    const std::vector<Vector> phi_theta_zero = {{0, 0, 0},
                                                {PI/3, 0, 0}, {PI/3, 2*PI/5, 0}, {PI/3, 4*PI/5, 0}, {PI/3, 6*PI/5, 0}, {PI/3, 8*PI/5, 0},
                                                {2*PI/3, PI/5, 0}, {2*PI/3, 3*PI/5, 0}, {2*PI/3, 5*PI/5, 0}, {2*PI/3, 7*PI/5, 0}, {2*PI/3, 9*PI/5, 0},
                                                {PI, 0, 0}};
    const std::vector<std::array<uint8_t, 3>> indices = {{0,1,2},{0,2,3},{0,3,4},{0,4,5},{0,5,1},
                                                         {2,1,6},{3,2,7},{4,3,8},{5,4,9},{1,5,10},
                                                         {1,10,6},{2,6,7},{3,7,8},{4,8,9},{5,9,10},
                                                         {11,6,10},{11,7,6},{11,8,7},{11,9,8},{11,10,9}};
    const std::vector<vec4> colors = {{0.12156862745098039, 0.4666666666666667, 0.7058823529411765, 1.0}, {1.0, 0.4980392156862745, 0.054901960784313725, 1.0}, {0.17254901960784313, 0.6274509803921569, 0.17254901960784313, 1.0}, {0.8392156862745098, 0.15294117647058825, 0.1568627450980392, 1.0}, {0.5803921568627451, 0.403921568627451, 0.7411764705882353, 1.0},
                                      {0.5490196078431373, 0.33725490196078434, 0.29411764705882354, 1.0}, {0.8901960784313725, 0.4666666666666667, 0.7607843137254902, 1.0}, {0.4980392156862745, 0.4980392156862745, 0.4980392156862745, 1.0}, {0.7372549019607844, 0.7411764705882353, 0.13333333333333333, 1.0}, {0.09019607843137255, 0.7450980392156863, 0.8117647058823529, 1.0},
                                      {0.12156862745098039, 0.4666666666666667, 0.7058823529411765, 1.0}, {1.0, 0.4980392156862745, 0.054901960784313725, 1.0}, {0.17254901960784313, 0.6274509803921569, 0.17254901960784313, 1.0}, {0.8392156862745098, 0.15294117647058825, 0.1568627450980392, 1.0}, {0.5803921568627451, 0.403921568627451, 0.7411764705882353, 1.0},
                                      {0.5490196078431373, 0.33725490196078434, 0.29411764705882354, 1.0}, {0.8901960784313725, 0.4666666666666667, 0.7607843137254902, 1.0}, {0.4980392156862745, 0.4980392156862745, 0.4980392156862745, 1.0}, {0.7372549019607844, 0.7411764705882353, 0.13333333333333333, 1.0}, {0.09019607843137255, 0.7450980392156863, 0.8117647058823529, 1.0}}; 
    // subdividing
    for (unsigned int i=0; i<indices.size(); i++)
    {
        vec4 color = colors[i];
        std::array<uint8_t, 3> indice = indices[i];
        Vector p0 = phi_theta_zero[indice[0]];
        Vector p1 = phi_theta_zero[indice[1]];
        Vector p2 = phi_theta_zero[indice[2]];
        Vector u = (p1-p0) / (divides + 1);
        Vector v = (p2-p0) / (divides + 1);
        /*
        Loop over strips of the triangle as such :
                                                   0
                                                  /x\
                                                 1---2
                                                /x\x/ \
                                       u ↙     3---4---5      ↘ v
                                              /x\x/ \ / \
                                             6---7---8---9    
                                            /x\x/ \ / \ / \ 
                                           10--11--12--13--14 
        
        */
        for (unsigned int j=0; j<divides+1; j++) // loop over v divides - the strips
        {
            // add the top of strip triangle
            Vector top = p0+j*v;
            faces.push_back(_phi_theta_to_face(top, top+u, top+v, color, radius));
            for (unsigned int k=0; k<j; k++)  // loop over u divides - the rectangles of a strip
            {
                top += u;
                // add faces linked to a rectangle such as 1-2-4-3, 
                faces.push_back(Face({top, top+u, top+v}, color));
                faces.push_back(Face({top, top+v, top+v-u}, color));
            }
        }
    }
    return faces;
}

Face Mesh::_phi_theta_to_face(const Vector& p0, const Vector& p1, const Vector& p2, vec4 color, double radius)
{
    return Face({Vector(radius*std::sin(p0.x)*std::cos(p0.y), radius*std::sin(p0.x)*std::sin(p0.y), radius*std::cos(p0.x)),
                 Vector(radius*std::sin(p1.x)*std::cos(p1.y), radius*std::sin(p1.x)*std::sin(p1.y), radius*std::cos(p1.x)),
                 Vector(radius*std::sin(p2.x)*std::cos(p2.y), radius*std::sin(p2.x)*std::sin(p2.y), radius*std::cos(p2.x))},
                 color);
}