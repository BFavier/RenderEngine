#pragma once
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/shaders/Type.hpp>

namespace RenderEngine
{
    class Vector
    {
    public:
        double x = 0.;
        double y = 0.;
        double z = 0.;
    public:
        Vector();
        Vector(const Vector& other);
        Vector(const std::initializer_list<double>& elements);
        Vector(double X, double Y, double Z);
        ~Vector();
        ///< Convert to a vec3
        vec3 to_vec3() const;
        //!< Returns the norm of a Vector
        double norm() const;
        ///< Return the vector with unit norm (or the null vector)
        Vector normed() const;
        ///< Return the vector projected on the given axis
        Vector projected(const Vector& axis) const;
        ///< Test if two vectors are equal, given a tolerance
        static bool equal(const Vector& vec1, const Vector& vec2, double tolerance=1.0E-6);
        ///< Cross product of two vector
        static Vector cross(const Vector& vec1, const Vector& vec2);
        ///< Dot product of two vector
        static double dot(const Vector& vec1, const Vector& vec2);
        ///< Signed angle between two vectors. Such that v2 is v1 rotated by 'angle' around Vector::cross(v1,v2) (if it's norm is superior to 0)
        static double angle(const Vector& v1, const Vector& v2);
        Vector& operator=(const Vector& other);
        Vector operator-() const;
        friend std::ostream& operator<<(std::ostream& os, const Vector& vec);
        Vector operator+(const Vector& other) const;
        Vector operator-(const Vector& other) const;
        Vector operator*(const Vector& other) const;
        Vector operator*(const double other) const;
        friend Vector operator*(double other, const Vector& vec);
        Vector operator/(const Vector& other) const;
        Vector operator/(const double other) const;
        friend Vector operator/(double other, const Vector& vec);
        Vector& operator+=(const Vector& other);
        Vector& operator-=(const Vector& other);
        Vector& operator*=(const Vector& other);
        Vector& operator*=(const double other);
        Vector& operator/=(const Vector& other);
        Vector& operator/=(const double other);
    };
}
