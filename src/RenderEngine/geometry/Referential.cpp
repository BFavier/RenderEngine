#include <RenderEngine/geometry/Referential.hpp>
using namespace RenderEngine;

Referential::Referential(const Vector& _position, const Quaternion _orientation, double _scale, Referential* _parent)
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

Referential::Referential(const Referential& other)
{
    *this = other;
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
        parent = nullptr;
    }
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
    std::tuple<Vector, Quaternion, double> this_coordinates = absolute_coordinates();
    std::tuple<Vector, Quaternion, double> other_coordinates = other.absolute_coordinates();
    Quaternion inverse_orientation = std::get<1>(other_coordinates).inverse();
    return std::make_tuple(inverse_orientation * (std::get<0>(this_coordinates) - std::get<0>(other_coordinates)) / std::get<2>(other_coordinates),
                           inverse_orientation * std::get<1>(this_coordinates),
                           std::get<2>(this_coordinates) / std::get<2>(other_coordinates));
}
