#pragma once
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/graphics/shaders/Types.hpp>
#include <array>

namespace RenderEngine
{
    class Vector;
    class Quaternion;

    class Matrix
    {
    public:
        Matrix();
        Matrix(const Matrix& other);
        Matrix(const Vector& diagonal);
        Matrix(const std::array<std::array<double, 3>, 3>& elements);
        Matrix(const Quaternion& quat);
        ~Matrix();
    public:
        std::array<std::array<double, 3>, 3> scalars = {{{0., 0., 0.}, {0., 0., 0.}, {0., 0., 0.}}};
    public:
        Matrix transposed() const;
        mat3 to_mat3() const;
        Matrix operator*(const Matrix& other) const;
        Vector operator*(const Vector& vector) const;
        Matrix& operator=(const Matrix& other);
        friend std::ostream& operator<<(std::ostream& os, const Matrix& mat);
    };
}
