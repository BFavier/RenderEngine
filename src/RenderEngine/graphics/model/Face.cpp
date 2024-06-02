#include <RenderEngine/graphics/model/Face.hpp>
using namespace RenderEngine;


Face::Face(const std::array<Vector, 3>& _points, const vec4& _color)
{
    points = _points;
    color = _color;
    normal = Vector::cross((_points[1] - _points[0]), (_points[2] - _points[0])).normed();
};

std::vector<Face> Face::quad(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, const vec4& color)
{
    return {Face({p1, p2, p3}, color), Face({p1, p3, p4}, color)};
}

std::vector<Face> Face::cube(double length, const std::optional<vec4>& color)
{
    double l = length/2;
    std::vector<Face> faces = { Face({{{-l, -l, -l}, {-l, l, -l}, {l, l, -l}}}, color.has_value() ? color.value() : vec4({1., 0., 0., 1.})),
                               Face({{{-l, -l, -l}, {l, l, -l}, {l, -l, -l}}}, color.has_value() ? color.value() : vec4({1., 0., 0., 1.})),
                               Face({{{-l, -l, l}, {l, l, l}, {-l, l, l}}}, color.has_value() ? color.value() : vec4({1., 1., 0., 1.})),
                               Face({{{-l, -l, l}, {l, -l, l}, {l, l, l}}}, color.has_value() ? color.value() : vec4({1., 1., 0., 1.})),

                               Face({{{-l, -l, -l}, {l, -l, -l}, {l, -l, l}}}, color.has_value() ? color.value() : vec4({1., 0., 1., 1.})),
                               Face({{{-l, -l, -l}, {l, -l, l}, {-l, -l, l}}}, color.has_value() ? color.value() : vec4({1., 0., 1., 1.})),
                               Face({{{-l, l, -l}, {l, l, l}, {l, l, -l}}}, color.has_value() ? color.value() : vec4({0., 0., 1., 1.})),
                               Face({{{-l, l, -l}, {-l, l, l}, {l, l, l}}}, color.has_value() ? color.value() : vec4({0., 0., 1., 1.})),
                               
                               Face({{{-l, -l, -l}, {-l, -l, l}, {-l, l, l}}}, color.has_value() ? color.value() : vec4({0., 1., 1., 1.})),
                               Face({{{-l, -l, -l}, {-l, l, l}, {-l, l, -l}}}, color.has_value() ? color.value() : vec4({0., 1., 1., 1.})),
                               Face({{{l, -l, -l}, {l, l, l}, {l, -l, l}}}, color.has_value() ? color.value() : vec4({0., 1., 0., 1.})),
                               Face({{{l, -l, -l}, {l, l, -l}, {l, l, l}}}, color.has_value() ? color.value() : vec4({0., 1., 0., 1.}))};
    return faces;
}

std::vector<Face> Face::sphere(double radius, uint32_t divides, const std::optional<vec4>& color)
{
    std::vector<Face> faces;
    // creating the icosahedron coordinates
    const double phi = (1.0+std::sqrt(5))/2.0;
    const double phi_square = std::sqrt(1 + std::pow(phi, 2));
    const std::vector<Vector> phi_theta_r = {{0, 0, radius},
                                             {PI/3, 0, radius}, {PI/3, 2*PI/5, radius}, {PI/3, 4*PI/5, radius}, {PI/3, 6*PI/5, radius}, {PI/3, 8*PI/5, radius},
                                             {2*PI/3, PI/5, radius}, {2*PI/3, 3*PI/5, radius}, {2*PI/3, 5*PI/5, radius}, {2*PI/3, 7*PI/5, radius}, {2*PI/3, 9*PI/5, radius},
                                             {PI, 0, radius}};
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
        vec4 face_color = colors[i];
        std::array<uint8_t, 3> indice = indices[i];
        Vector p0 = _spherical_to_cartesian(phi_theta_r[indice[0]]);
        Vector p1 = _spherical_to_cartesian(phi_theta_r[indice[1]]);
        Vector p2 = _spherical_to_cartesian(phi_theta_r[indice[2]]);
        Vector u = (p1-p0) / (divides + 1);
        Vector v = (p2-p0) / (divides + 1);
        /*
        Loop over strips of the triangle as such :
                                                   0
                                                  /x\
                                                 1---2
                                                /x\x/ \
                                        u ↙    3---4---5   ↘ v
                                              /x\x/ \ / \
                                             6---7---8---9    
                                            /x\x/ \ / \ / \ 
                                           10--11--12--13--14 
        
        */
        for (unsigned int j=0; j<divides+1; j++) // loop over v divides - the strips
        {
            // add the top of strip triangle, such as 0-1-2
            Vector top = p0+j*v;
            faces.push_back(Face({top.normed()*radius, (top+u).normed()*radius, (top+v).normed()*radius}, color.has_value() ? color.value() : face_color));
            for (unsigned int k=0; k<divides-j; k++)  // loop over u divides - the rectangles of a strip
            {
                top += u;
                // add faces linked to a rectangle, such as 1-2-4-3
                faces.push_back(Face({top.normed()*radius, (top+u).normed()*radius, (top+v).normed()*radius}, color.has_value() ? color.value() : face_color));
                faces.push_back(Face({top.normed()*radius, (top+v).normed()*radius, (top+v-u).normed()*radius}, color.has_value() ? color.value() : face_color));
            }
        }
    }
    return faces;
}

std::vector<Face> Face::cylinder(double length, double radius, uint32_t divides, const std::optional<vec4>& color)
{
    const std::vector<vec4> colors = {{0.21568627450980393, 0.49411764705882355, 0.7215686274509804, 1.0}, {0.30196078431372547, 0.6862745098039216, 0.2901960784313726, 1.0}, {0.596078431372549, 0.3058823529411765, 0.6392156862745098, 1.0}, {1.0, 0.4980392156862745, 0.0, 1.0}, {1.0, 1.0, 0.2, 1.0}, {0.6509803921568628, 0.33725490196078434, 0.1568627450980392, 1.0}, {0.9686274509803922, 0.5058823529411764, 0.7490196078431373, 1.0}};
    Vector z(0., 0., length);
    std::vector<Face> faces;
    for (uint32_t i=0; i<divides;i++)
    {
        double angle1 = (2*PI*i)/divides;
        double angle2 = (2*PI*(i+1))/divides;
        Vector p1(radius*std::cos(angle1), radius*std::sin(angle1), 0.0);
        Vector p2(radius*std::cos(angle2), radius*std::sin(angle2), 0.0);
        faces.push_back(Face({p1, Vector(0., 0., 0.), p2}, color.has_value() ? color.value() : vec4({0.8941176470588236, 0.10196078431372549, 0.10980392156862745, 1.0})));
        faces.push_back(Face({p2+z, z, p1+z}, color.has_value() ? color.value() : vec4({0.6, 0.6, 0.6, 1.0})));
        faces.push_back(Face({p1+z, p1, p2}, color.has_value() ? color.value() : colors[i % colors.size()]));
        faces.push_back(Face({p1+z, p2, p2+z}, color.has_value() ? color.value() : colors[i % colors.size()]));
    }
    return faces;
}

std::vector<Face> Face::cone(double length, double radius, uint32_t divides, const std::optional<vec4>& color)
{
    const std::vector<vec4> colors = {{0.21568627450980393, 0.49411764705882355, 0.7215686274509804, 1.0}, {0.30196078431372547, 0.6862745098039216, 0.2901960784313726, 1.0}, {0.596078431372549, 0.3058823529411765, 0.6392156862745098, 1.0}, {1.0, 0.4980392156862745, 0.0, 1.0}, {1.0, 1.0, 0.2, 1.0}, {0.6509803921568628, 0.33725490196078434, 0.1568627450980392, 1.0}, {0.9686274509803922, 0.5058823529411764, 0.7490196078431373, 1.0}};
    Vector z(0., 0., length);
    std::vector<Face> faces;
    for (uint32_t i=0; i<divides;i++)
    {
        double angle1 = (2*PI*i)/divides;
        double angle2 = (2*PI*(i+1))/divides;
        Vector p1(radius*std::cos(angle1), radius*std::sin(angle1), 0.0);
        Vector p2(radius*std::cos(angle2), radius*std::sin(angle2), 0.0);
        faces.push_back(Face({p1, Vector(0., 0., 0.), p2}, color.has_value() ? color.value() : vec4({0.8941176470588236, 0.10196078431372549, 0.10980392156862745, 1.0})));
        faces.push_back(Face({z, p1, p2}, color.has_value() ? color.value() : colors[i % colors.size()]));
    }
    return faces;
}

Vector Face::_spherical_to_cartesian(const Vector& p)
{
    return Vector(p.z*std::sin(p.x)*std::cos(p.y), p.z*std::sin(p.x)*std::sin(p.y), p.z*std::cos(p.x));
}
