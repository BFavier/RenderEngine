#include <RenderEngine/geometry/Vector.hpp>
#include <vector>
using namespace RenderEngine;

Vector::Vector()
{
    x = static_cast<double>(0);
    y = static_cast<double>(0);
    z = static_cast<double>(0);
}

Vector::Vector(const Vector& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
}

Vector::Vector(const std::initializer_list<double>& elements)
{
    std::vector<double> coordinates(elements);
    x = coordinates[0];
    y = coordinates[1];
    z = coordinates[2];
}

Vector::Vector(double X, double Y, double Z)
{
    x = X;
    y = Y;
    z = Z;
}

Vector::~Vector()
{
}

vec3 Vector::to_vec3() const
{
    return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
}

double Vector::norm() const
{
    return std::sqrt(std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));
}

Vector Vector::normed() const
{
    double n = norm();
    if (n > 0.)
    {
        return *this / n;
    }
    return Vector(0., 0., 0.);
}

bool Vector::equal(const Vector& vec1, const Vector& vec2, double tolerance)
{
    double n = (vec1-vec2).norm();
	if (n < tolerance)
	{
		return true;
	}
	return false;
}

Vector Vector::cross(const Vector& vec1, const Vector& vec2)
{
    return Vector(vec1.y*vec2.z-vec2.y*vec1.z, vec1.z*vec2.x-vec2.z*vec1.x, vec1.x*vec2.y-vec2.x*vec1.y);
}

double Vector::dot(const Vector& vec1, const Vector& vec2)
{
    return vec1.x*vec2.x+vec1.y*vec2.y+vec1.z*vec2.z;
}

double Vector::angle(const Vector& v1, const Vector& v2)
{
    Vector u1 = v1.normed();
    Vector u2 = v2.normed();
    double sinus = Vector::cross(u1, u2).norm();
    double cosinus = Vector::dot(u1, u2);
    double radians = atan2(sinus, cosinus);
    return radians * 180. / PI;
}

Vector Vector::projected(const Vector& axis) const
{
    Vector normed_axis = axis.normed();
    return Vector::dot(*this, normed_axis) * normed_axis;
}

Vector& Vector::operator=(const Vector& other)
{
    x=other.x;
    y=other.y;
    z=other.z;
    return *this;
}

Vector Vector::operator-() const
{
    return Vector(-x, -y, -z);
}

namespace RenderEngine
{
    std::ostream& operator<<(std::ostream& os, const Vector& vec)
    {
        os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
        return os;
    }
}

Vector Vector::operator+(const Vector& other) const
{
    Vector vector(*this);
    return vector += other;
}

Vector Vector::operator-(const Vector& other) const
{
    Vector vector(*this);
    return vector -= other;
}

Vector Vector::operator*(const Vector& other) const
{
    Vector vector(*this);
    return vector *= other;
}

Vector Vector::operator*(const double other) const
{
    Vector vector(*this);
    return vector *= other;
}

namespace RenderEngine
{
    Vector operator*(const double other, const Vector& vec)
    {
        Vector vector(vec);
        return vector *= other;
    }
}

Vector Vector::operator/(const Vector& other) const
{
    Vector vector(*this);
    return vector /= other;
}

Vector Vector::operator/(const double other) const
{
    Vector vector(*this);
    return vector /= other;
}

namespace RenderEngine
{
    Vector operator/(const double other, const Vector& vec)
    {
        return Vector(other/vec.x, other/vec.y, other/vec.z);
    }
}

Vector& Vector::operator+=(const Vector& other)
{
    x+=other.x;
    y+=other.y;
    z+=other.z;
    return *this;
}

Vector& Vector::operator-=(const Vector& other)
{
    x-=other.x;
    y-=other.y;
    z-=other.z;
    return *this;
}

Vector& Vector::operator*=(const Vector& other)
{
    x*=other.x;
    y*=other.y;
    z*=other.z;
    return *this;
}

Vector& Vector::operator*=(const double other)
{
    x*=other;
    y*=other;
    z*=other;
    return *this;
}

Vector& Vector::operator/=(const Vector& other)
{
    x/=other.x;
    y/=other.y;
    z/=other.z;
    return *this;
}

Vector& Vector::operator/=(const double other)
{
    x/=other;
    y/=other;
    z/=other;
    return *this;
}
