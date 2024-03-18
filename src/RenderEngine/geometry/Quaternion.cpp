#include <RenderEngine/geometry/Quaternion.hpp>
#include <RenderEngine/geometry/Vector.hpp>

using namespace RenderEngine;

Quaternion::Quaternion()
{
}

Quaternion::Quaternion(const Quaternion& other)
{
    w = other.w;
    x = other.x;
    y = other.y;
    z = other.z;
}

Quaternion::Quaternion(double W, double X, double Y, double Z)
{
    w = W;
    x = X;
    y = Y;
    z = Z;
}

Quaternion::Quaternion(double angle, const Vector& axis)
{
    double radians = angle * PI / 180.;
    Vector normed = axis.normed();
    w = cos(radians/2);
    x = normed.x*sin(radians/2);
    y = normed.y*sin(radians/2);
    z = normed.z*sin(radians/2);
}

Quaternion::Quaternion(const Vector& from, const Vector& to)
{
    double radians = Vector::angle(from, to) * PI / 180.;
    Vector axis = Vector::cross(from, to).normed();
    //If 'from' is in the opposite direction of 'to', they are rotated by 180°, and there is an infinite number of axis to choose from.
    if (axis.norm() < 1.0E-6 && std::abs(radians) > 1.0E-3)
    {
        Vector n1 = Vector::cross(from, {1., 0., 0.}).normed();
        if (n1.norm() > 1.0E-6)
        {
            axis = Vector::cross(from, n1).normed();
        }
        else
        {
            n1 = Vector::cross(from, {0., 1., 0.});
            axis = Vector::cross(from, n1).normed();
        }
    }
    w = cos(radians/2);
    x = axis.x*sin(radians/2);
    y = axis.y*sin(radians/2);
    z = axis.z*sin(radians/2);
}

Quaternion::~Quaternion()
{
}

double Quaternion::norm() const
{
    return std::sqrt(std::pow(w, 2) + std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2));
}

void Quaternion::normalize()
{
    double n = norm();
    if (n > 0.)
    {
        *this /= n;
    }
}

Quaternion Quaternion::normed() const
{
    double n = norm();
    if (n > 0.)
    {
        return *this / (n*n);
    }
    return Quaternion(0., 0., 0., 0.);
}

Quaternion Quaternion::conjugate() const
{
    return Quaternion(w, -x, -y, -z);
}

Quaternion Quaternion::inverse() const
{
    double squared_norm = std::pow(w, 2) + std::pow(x, 2) + std::pow(y, 2) + std::pow(z, 2);
    return conjugate()/squared_norm;
}

bool Quaternion::equal(const Quaternion& q1, const Quaternion& q2, double tolerance)
{
    double n = (q1-q2).norm();
	if (n < tolerance)
	{
		return true;
	}
	return false;
}

double Quaternion::dot(const Quaternion& q1, const Quaternion& q2)
{
    return q1.w*q2.w + q1.x*q2.x + q1.y*q2.y + q1.z*q2.z;
}

Quaternion Quaternion::SLERP(const Quaternion& Q1, const Quaternion& Q2, double t)
{
    t = std::max(0., std::min(t, 1.));
    Quaternion q1(Q1.normed());
    Quaternion q2(Q2.normed());
    double dot = Quaternion::dot(q1, q2);
    if (dot > 0.9999)
    {
        Quaternion q = q1*(1.-t) + q2*t;
        return q.normed();
    }
    if (dot < 0.)
    {
        q2 = -q2;
        dot = -dot;
    }
    double theta = acos(dot);
    double s1 = sin(theta*(1.-t))/sin(theta);
    double s2 = sin(theta*t)/sin(theta);
    return s1*q1 + s2*q2;
}

namespace RenderEngine
{
    std::ostream& operator<<(std::ostream& os, const Quaternion& rot)
    {
        os << "(" << rot.w << ", " << rot.x << ", " << rot.y << ", " << rot.z << ")";
        return os;
    }
}

Quaternion& Quaternion::operator=(const Quaternion& other)
{
    w = other.w;
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
    Vector u1(x, y, z);
    Vector u2(other.x, other.y, other.z);
    Vector u = w*u2 + other.w*u1 + Vector::cross(u1, u2);
    return Quaternion(w*other.w - Vector::dot(u1, u2), u.x, u.y, u.z);
}

Vector Quaternion::operator*(const Vector& vector) const
{
    Quaternion result(*this * Quaternion(0., vector.x, vector.y, vector.z) * inverse());
    return Vector(result.x, result.y, result.z);
}

Quaternion Quaternion::operator*(double other) const
{
    return Quaternion(w*other, x*other, y*other, z*other);
}

namespace RenderEngine
{
    Quaternion operator*(double other, const Quaternion& quat)
    {
            return Quaternion(quat.w*other, quat.x*other, quat.y*other, quat.z*other);
    }
}

Quaternion& Quaternion::operator*=(double other)
{
    w *= other;
    x *= other;
    y *= other;
    z *= other;
    return *this;
}

Quaternion Quaternion::operator/(double other) const
{
    return Quaternion(w/other, x/other, y/other, z/other);
}

Quaternion& Quaternion::operator/=(double other)
{
    w /= other;
    x /= other;
    y /= other;
    z /= other;
    return *this;
}

Quaternion Quaternion::operator+(const Quaternion& other) const
{
    return Quaternion(w+other.w, x+other.x, y+other.y, z+other.z);
}

Quaternion& Quaternion::operator+=(const Quaternion& other)
{
    w += other.w;
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Quaternion Quaternion::operator-(const Quaternion& other) const
{
    return Quaternion(w-other.w, x-other.x, y-other.y, z-other.z);
}

Quaternion Quaternion::operator-() const
{
    return Quaternion(-w, -x, -y, -z);
}

Quaternion& Quaternion::operator-=(const Quaternion& other)
{
    w -= other.w;
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}
