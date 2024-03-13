#include <RenderEngine/geometry/Referential.hpp>
using namespace RenderEngine;

Referential::Referential()
{
}

Referential::Referential(const Referential& other)
{
    *this = other;
}

Referential::~Referential()
{
}

Referential& Referential::operator=(const Referential& other)
{
    position = other.position;
    orientation = other.orientation;
    return *this;
}
