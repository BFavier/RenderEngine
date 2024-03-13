#include <RenderEngine/geometry/Matrix.hpp>
#include <RenderEngine/geometry/Vector.hpp>
#include <RenderEngine/geometry/Quaternion.hpp>
using namespace RenderEngine;

Matrix::Matrix()
{
}

Matrix::Matrix(const Matrix& other)
{
    *this = other;
}

Matrix::Matrix(const Vector& diagonal)
{
    scalars[0][0] = diagonal.x;
    scalars[1][1] = diagonal.y;
    scalars[2][2] = diagonal.z;
}

Matrix::Matrix(const std::array<std::array<double, 3>, 3>& elements)
{
    scalars = elements;
}

Matrix::Matrix(const Quaternion& quat)
{
    scalars[0][0] = std::pow(quat.w, 2.) + std::pow(quat.x, 2.) - std::pow(quat.y, 2.) - std::pow(quat.z, 2.);
    scalars[0][1] = 2*quat.x*quat.y - 2*quat.w*quat.z;
    scalars[0][2] = 2*quat.w*quat.y + 2*quat.x*quat.z;

    scalars[1][0] = 2*quat.w*quat.z + 2*quat.x*quat.y;
    scalars[1][1] = std::pow(quat.w, 2.) - std::pow(quat.x, 2.) + std::pow(quat.y, 2.) - std::pow(quat.z, 2.);
    scalars[1][2] = 2*quat.y*quat.z - 2*quat.w*quat.x;

    scalars[2][0] = 2*quat.x*quat.z - 2*quat.w*quat.y;
    scalars[2][1] = 2*quat.w*quat.x + 2*quat.y*quat.z;
    scalars[2][2] = std::pow(quat.w, 2.) - std::pow(quat.x, 2.) - std::pow(quat.y, 2.) + std::pow(quat.z, 2.);
}

Matrix::~Matrix()
{
}

Matrix Matrix::transposed() const
{
    Matrix result;
    for (unsigned int i=0; i<3; i++)
    {
        for (unsigned int j=0; j<3; j++)
        {
            result.scalars[i][j] = scalars[i][j];
        }
    }
    return result;
}

mat3 Matrix::to_mat3() const
{
    return {static_cast<float>(scalars[0][0]),
            static_cast<float>(scalars[0][1]),
            static_cast<float>(scalars[0][2]),
            0.f,
            static_cast<float>(scalars[1][0]),
            static_cast<float>(scalars[1][1]),
            static_cast<float>(scalars[1][2]),
            0.f,
            static_cast<float>(scalars[2][0]),
            static_cast<float>(scalars[2][1]),
            static_cast<float>(scalars[2][2]),
            0.f};
}

Matrix Matrix::operator*(const Matrix& other) const
{
    Matrix result;
    for (unsigned int i=0; i<3; i++)
    {
        for (unsigned int j=0; j<3; j++)
        {
            for (unsigned int k=0; k<3; k++)
            {
                result.scalars[i][j] += scalars[i][k] * other.scalars[k][j];
            }
        }
    }
    return result;
}

Vector Matrix::operator*(const Vector& vector) const
{
    return {vector.x * scalars[0][0] + vector.y * scalars[0][1] + vector.z * scalars[0][2],
            vector.x * scalars[1][0] + vector.y * scalars[1][1] + vector.z * scalars[1][2],
            vector.x * scalars[2][0] + vector.y * scalars[2][1] + vector.z * scalars[2][2]};
}

Matrix& Matrix::operator=(const Matrix& other)
{
    scalars = other.scalars;
    return *this;
}

namespace RenderEngine
{
    std::ostream& operator<<(std::ostream& os, const Matrix& mat)
    {
        for (unsigned int i=0; i<3; i++)
        {
            os << "\n{";
            for (unsigned int j=0; j<3; j++)
            {
                os << mat.scalars[i][j];
                if (j < 2)
                {
                    os << ",\t";
                }
                else
                {
                    os << "}";
                }
            }
        }
        os << std::endl;
        return os;
    }
}
