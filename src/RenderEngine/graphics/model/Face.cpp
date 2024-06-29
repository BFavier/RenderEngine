#include <RenderEngine/graphics/model/Face.hpp>
using namespace RenderEngine;


Face::Face(const std::array<Vector, 3>& _points, const std::array<UV, 3>& _UVs, const Color& _color, const Material& _material)
{
    points = _points;
    UVs = _UVs;
    color = _color;
    Vector normal = Vector::cross((_points[1] - _points[0]), (_points[2] - _points[0])).normed();
    normals = {normal, normal, normal};
    material = _material;
}

Face::~Face()
{
}

std::vector<Face> Face::quad(const Vector& p1, const Vector& p2, const Vector& p3, const Vector& p4, const Color& color, const Material& material)
{
    return {Face({p1, p2, p3}, {UV(0., 0.), UV(0., 1.), UV(1., 1.)}, color, material),
            Face({p1, p3, p4}, {UV(0., 0.), UV(1., 1.), UV(1., 0.)}, color, material)};
}

std::vector<Face> Face::cube(double length, const std::optional<Color>& color, const Material& material)
{
    double l = length/2;
    std::vector<Face> faces = {Face({{{-l, -l, -l}, {-l, l, -l}, {l, l, -l}}}, {UV(0., 0.), UV(0., 1.), UV(1., 1.)}, color.has_value() ? color.value() : Color(1., 0., 0., 1.), material),
                               Face({{{-l, -l, -l}, {l, l, -l}, {l, -l, -l}}}, {UV(0., 0.), UV(1., 1.), UV(1., 0.)}, color.has_value() ? color.value() : Color(1., 0., 0., 1.), material),
                               Face({{{-l, -l, l}, {l, l, l}, {-l, l, l}}}, {UV(0., 0.), UV(0., 1.), UV(1., 1.)}, color.has_value() ? color.value() : Color(1., 1., 0., 1.), material),
                               Face({{{-l, -l, l}, {l, -l, l}, {l, l, l}}}, {UV(0., 0.), UV(1., 1.), UV(1., 0.)}, color.has_value() ? color.value() : Color(1., 1., 0., 1.), material),

                               Face({{{-l, -l, -l}, {l, -l, -l}, {l, -l, l}}}, {UV(0., 0.), UV(0., 1.), UV(1., 1.)}, color.has_value() ? color.value() : Color(1., 0., 1., 1.), material),
                               Face({{{-l, -l, -l}, {l, -l, l}, {-l, -l, l}}}, {UV(0., 0.), UV(1., 1.), UV(1., 0.)}, color.has_value() ? color.value() : Color(1., 0., 1., 1.), material),
                               Face({{{-l, l, -l}, {l, l, l}, {l, l, -l}}}, {UV(0., 0.), UV(0., 1.), UV(1., 1.)}, color.has_value() ? color.value() : Color(0., 0., 1., 1.), material),
                               Face({{{-l, l, -l}, {-l, l, l}, {l, l, l}}}, {UV(0., 0.), UV(1., 1.), UV(1., 0.)}, color.has_value() ? color.value() : Color(0., 0., 1., 1.), material),
                               
                               Face({{{-l, -l, -l}, {-l, -l, l}, {-l, l, l}}}, {UV(0., 0.), UV(0., 1.), UV(1., 1.)}, color.has_value() ? color.value() : Color(0., 1., 1., 1.), material),
                               Face({{{-l, -l, -l}, {-l, l, l}, {-l, l, -l}}}, {UV(0., 0.), UV(1., 1.), UV(1., 0.)}, color.has_value() ? color.value() : Color(0., 1., 1., 1.), material),
                               Face({{{l, -l, -l}, {l, l, l}, {l, -l, l}}}, {UV(0., 0.), UV(0., 1.), UV(1., 1.)}, color.has_value() ? color.value() : Color(0., 1., 0., 1.), material),
                               Face({{{l, -l, -l}, {l, l, -l}, {l, l, l}}}, {UV(0., 0.), UV(1., 1.), UV(1., 0.)}, color.has_value() ? color.value() : Color(0., 1., 0., 1.), material)};
    return faces;
}

std::vector<Face> Face::sphere(double radius, uint32_t divides, bool smooth_normals, const std::optional<Color>& color, const Material& material)
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
    const std::vector<Color> colors = {{0.12156862745098039, 0.4666666666666667, 0.7058823529411765, 1.0}, {1.0, 0.4980392156862745, 0.054901960784313725, 1.0}, {0.17254901960784313, 0.6274509803921569, 0.17254901960784313, 1.0}, {0.8392156862745098, 0.15294117647058825, 0.1568627450980392, 1.0}, {0.5803921568627451, 0.403921568627451, 0.7411764705882353, 1.0},
                                       {0.5490196078431373, 0.33725490196078434, 0.29411764705882354, 1.0}, {0.8901960784313725, 0.4666666666666667, 0.7607843137254902, 1.0}, {0.4980392156862745, 0.4980392156862745, 0.4980392156862745, 1.0}, {0.7372549019607844, 0.7411764705882353, 0.13333333333333333, 1.0}, {0.09019607843137255, 0.7450980392156863, 0.8117647058823529, 1.0},
                                       {0.12156862745098039, 0.4666666666666667, 0.7058823529411765, 1.0}, {1.0, 0.4980392156862745, 0.054901960784313725, 1.0}, {0.17254901960784313, 0.6274509803921569, 0.17254901960784313, 1.0}, {0.8392156862745098, 0.15294117647058825, 0.1568627450980392, 1.0}, {0.5803921568627451, 0.403921568627451, 0.7411764705882353, 1.0},
                                       {0.5490196078431373, 0.33725490196078434, 0.29411764705882354, 1.0}, {0.8901960784313725, 0.4666666666666667, 0.7607843137254902, 1.0}, {0.4980392156862745, 0.4980392156862745, 0.4980392156862745, 1.0}, {0.7372549019607844, 0.7411764705882353, 0.13333333333333333, 1.0}, {0.09019607843137255, 0.7450980392156863, 0.8117647058823529, 1.0}}; 
    // subdividing
    for (unsigned int i=0; i<indices.size(); i++)
    {
        Color face_color = colors[i];
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
            Vector M0 = top.normed()*radius;
            Vector M1 = (top+u).normed()*radius;
            Vector M2 = (top+v).normed()*radius;
            Vector RTP0 = Face::_cartesian_to_spherical(M0);
            Vector RTP1 = Face::_cartesian_to_spherical(M1);
            Vector RTP2 = Face::_cartesian_to_spherical(M2);
            faces.push_back(Face({M0, M1, M2},
                                 {UV(RTP0.y, RTP0.z), UV(RTP1.y, RTP1.z), UV(RTP2.y, RTP2.z)},
                                 color.has_value() ? color.value() : face_color,
                                 material));
            for (unsigned int k=0; k<divides-j; k++)  // loop over u divides - the rectangles of a strip
            {
                top += u;
                Vector m0 = top.normed()*radius;
                Vector m1 = (top+u).normed()*radius;
                Vector m2 = (top+v).normed()*radius;
                Vector m3 = (top+v-u).normed()*radius;
                Vector rtp0 = Face::_cartesian_to_spherical(m0);
                Vector rtp1 = Face::_cartesian_to_spherical(m1);
                Vector rtp2 = Face::_cartesian_to_spherical(m2);
                Vector rtp3 = Face::_cartesian_to_spherical(m3);
                std::array<UV, 3> uv1 = {UV(rtp0.y, rtp0.z), UV(rtp1.y, rtp1.z), UV(rtp2.y, rtp2.z)};
                std::array<UV, 3> uv2 = {UV(rtp0.y, rtp0.z), UV(rtp2.y, rtp2.z), UV(rtp3.y, rtp3.z)};
                // add faces linked to a rectangle, such as 1-2-4-3
                faces.push_back(Face({m0, m1, m2}, uv1, color.has_value() ? color.value() : face_color, material));
                faces.push_back(Face({m0, m2, m3}, uv2, color.has_value() ? color.value() : face_color, material));
            }
        }
    }
    if (smooth_normals)
    {
        for (Face& face : faces)
        {
            face.normals = {face.points[0].normed(), face.points[1].normed(), face.points[2].normed()};
        }
    }
    return faces;
}

std::vector<Face> Face::cylinder(double length, double radius, uint32_t divides, bool smooth_normals, const std::optional<Color>& color, const Material& material)
{
    const std::vector<Color> colors = {{0.22, 0.49, 0.72, 1.0}, {0.30, 0.69, 0.29, 1.0}, {0.60, 0.31, 0.64, 1.0}, {1.0, 0.50, 0.0, 1.0}, {1.0, 1.0, 0.2, 1.0}, {0.65, 0.34, 0.16, 1.0}, {0.97, 0.51, 0.75, 1.0}};
    Vector z(0., 0., length);
    std::vector<Face> faces;
    for (uint32_t i=0; i<divides;i++)
    {
        double angle1 = (2*PI*i)/divides;
        double angle2 = (2*PI*(i+1))/divides;
        Vector p1(radius*std::cos(angle1), radius*std::sin(angle1), 0.0);
        Vector p2(radius*std::cos(angle2), radius*std::sin(angle2), 0.0);
        // sides
        Face face1({p1+z, p1, p2}, {UV(0.65, angle1/(2*PI)), UV(0.35, angle1/(2*PI)), UV(0.65, angle2/(2*PI))}, color.has_value() ? color.value() : colors[i % colors.size()], material);
        Face face2({p1+z, p2, p2+z}, {UV(0.65, angle1/(2*PI)), UV(0.35, angle2/(2*PI)), UV(0.65, angle2/(2*PI))}, color.has_value() ? color.value() : colors[i % colors.size()], material);
        if (smooth_normals)
        {
            Vector p1n = p1.normed();
            Vector p2n = p2.normed();
            face1.normals = {p1n, p1n, p2n};
            face1.normals = {p1n, p2n, p2n};
        }
        faces.push_back(face1);
        faces.push_back(face2);
        // circular extremities
        Vector uv1 = p1 / radius * Vector(0.29, 0.97, 0.);
        Vector uv2 = p2 / radius * Vector(0.29, 0.97, 0.);
        faces.push_back(Face({p1, Vector(0., 0., 0.), p2}, {UV(uv1.x + 0.15, uv1.y + 0.015), UV(0.15, 0.5), UV(uv2.x + 0.15, uv2.y + 0.015)}, color.has_value() ? color.value() : Color(0.89, 0.10, 0.11, 1.0), material));
        faces.push_back(Face({p2+z, z, p1+z}, {UV(uv2.x + 0.85, uv2.y + 0.015), UV(0.85, 0.5), UV(uv1.x + 0.85, uv1.y + 0.015)}, color.has_value() ? color.value() : Color(0.6, 0.6, 0.6, 1.0), material));
    }
    return faces;
}

std::vector<Face> Face::cone(double length, double radius, uint32_t divides, bool smooth_normals, const std::optional<Color>& color, const Material& material)
{
    const std::vector<Color> colors = {{0.22, 0.49, 0.72, 1.0}, {0.30, 0.69, 0.29, 1.0}, {0.60, 0.31, 0.64, 1.0}, {1.0, 0.50, 0.0, 1.0}, {1.0, 1.0, 0.2, 1.0}, {0.65, 0.34, 0.16, 1.0}, {0.97, 0.51, 0.75, 1.0}};
    Vector z(0., 0., length);
    std::vector<Face> faces;
    for (uint32_t i=0; i<divides;i++)
    {
        double angle1 = (2*PI*i)/divides;
        double angle2 = (2*PI*(i+1))/divides;
        Vector p1(radius*std::cos(angle1), radius*std::sin(angle1), 0.0);
        Vector p2(radius*std::cos(angle2), radius*std::sin(angle2), 0.0);
        Vector uv1 = p1 / radius * Vector(0.29, 0.97, 0.);
        Vector uv2 = p2 / radius * Vector(0.29, 0.97, 0.);
        faces.push_back(Face({p1, Vector(0., 0., 0.), p2}, {UV(uv1.x + 0.15, uv1.y + 0.015), UV(0.15, 0.5), UV(uv2.x + 0.15, uv2.y + 0.015)}, color.has_value() ? color.value() : Color(0.89, 0.10, 0.11, 1.0), material));
        Face face({z, p1, p2}, {UV(1.0, 0.5), UV(0.35, angle1/(2*PI)), UV(0.35, angle2/(2*PI))}, color.has_value() ? color.value() : colors[i % colors.size()], material);
        if (smooth_normals)
        {
            double angle = std::atan2(radius, length);
            face.normals = {_spherical_to_cartesian(Vector((angle1 + angle2)*0.5, angle, 1.0)),
                            _spherical_to_cartesian(Vector(angle1, angle, 1.0)),
                            _spherical_to_cartesian(Vector(angle2, angle, 1.0))};
        }
        faces.push_back(face);
    }
    return faces;
}

Vector Face::_spherical_to_cartesian(const Vector& p)
{
    return Vector(p.z*std::sin(p.x)*std::cos(p.y), p.z*std::sin(p.x)*std::sin(p.y), p.z*std::cos(p.x));
}

Vector Face::_cartesian_to_spherical(const Vector& p)
{
    double r = p.norm();
    double theta = std::atan2(std::sqrt(p.x*p.x + p.y*p.y), p.z);
    double phi = std::atan2(p.y, p.x);;
    return Vector(r, theta, phi);
}
