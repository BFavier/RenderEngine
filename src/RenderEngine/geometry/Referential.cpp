#include <RenderEngine/geometry/Referential.hpp>
using namespace RenderEngine;

Referential::Referential()
{
}

Referential::Referential(const Referential& other)
{
    *this = other;
}

Referential::Referential(Referential* _parent, const Vector& _position, const Quaternion _orientation, double _scale)
{
    parent = _parent;
    if (_parent != nullptr)
    {
        parent->childrens.push_back(this);
    }
    position = _position;
    orientation = _orientation;
    scale = _scale;
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
    std::tuple<Vector, Quaternion, double> coordinates;
    if (parent == nullptr)
    {
        coordinates = other.absolute_coordinates();
    }
    else
    {
        coordinates = other.coordinates_in(*parent);
    }
    std::tie(position, orientation, scale) = coordinates;
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

std::tuple<Vector, Quaternion, double> Referential::absolute_coordinates() const
{
    Vector abs_position = position;
    Quaternion abs_orientation = orientation;
    double abs_scale = scale;
    Referential* parent = this->parent;
    while (parent != nullptr)
    {
        abs_position = (parent->orientation.inverse() * abs_position) + parent->position;
        abs_orientation = parent->orientation * abs_orientation;
        abs_scale = abs_scale * parent->scale;
        parent = parent->parent;
    }
    return std::make_tuple(abs_position, abs_orientation, abs_scale);
}

std::tuple<Vector, Quaternion, double> Referential::coordinates_in(const Referential& other) const
{
    std::tuple<Vector, Quaternion, double> this_position = absolute_coordinates();
    std::tuple<Vector, Quaternion, double> other_position = other.absolute_coordinates();
    return std::make_tuple(std::get<0>(this_position) - std::get<0>(other_position),
                           std::get<1>(other_position) * std::get<1>(this_position).inverse(),
                           std::get<2>(this_position) / std::get<2>(other_position));
}
