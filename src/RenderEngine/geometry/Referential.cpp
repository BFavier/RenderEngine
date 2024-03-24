#include <RenderEngine/geometry/Referential.hpp>
using namespace RenderEngine;

Referential::Referential()
{
}

Referential::Referential(const Referential& other)
{
    *this = other;
}

Referential::Referential(Referential* _parent, const Vector& _position, const Quaternion _orientation)
{
    parent = _parent;
    if (_parent != nullptr)
    {
        parent->childrens.push_back(this);
    }
    position = _position;
    orientation = _orientation;
}

Referential::~Referential()
{
    for (Referential* child : childrens)
    {
        child->position = orientation.inverse() * child->position + position;
        child->orientation = child->orientation * orientation;
        child->parent = parent;
    }
    if (parent != nullptr)
    {
        parent->childrens.remove(this);
    }
}

Referential& Referential::operator=(const Referential& other)
{
    std::pair<Vector, Quaternion> coordinates;
    if (parent == nullptr)
    {
        coordinates = other.absolute_coordinates();
    }
    else
    {
        coordinates = other.coordinates_in(*parent);
    }
    position = coordinates.first;
    orientation = coordinates.second;
    return *this;
}

void Referential::detach()
{
    if (parent != nullptr)
    {
        parent->childrens.remove(this);
    }
    parent = nullptr;
}

void Referential::attach(Referential& _parent)
{
    detach();
    parent = &_parent;
    _parent.childrens.push_back(this);
}

std::pair<Vector, Quaternion> Referential::absolute_coordinates() const
{
    Vector abs_position = position;
    Quaternion abs_orientation = orientation;
    Referential* parent = this->parent;
    while (parent != nullptr)
    {
        abs_position = (parent->orientation.inverse() * abs_position) + parent->position;
        abs_orientation = parent->orientation * abs_orientation;
        parent = parent->parent;
    }
    return {abs_position, abs_orientation};
}

std::pair<Vector, Quaternion> Referential::coordinates_in(const Referential& other) const
{
    std::pair<Vector, Quaternion> this_position = absolute_coordinates();
    std::pair<Vector, Quaternion> other_position = other.absolute_coordinates();
    return {this_position.first - other_position.first, other_position.second * this_position.second.inverse()};
}
