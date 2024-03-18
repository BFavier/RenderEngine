#pragma once
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/geometry/Vector.hpp>
#include <RenderEngine/geometry/Quaternion.hpp>
#include <list>

namespace RenderEngine
{
    class Referential
    {
    public:
        Referential();
        Referential(Referential* parent, const Vector& position, const Quaternion orientation);
        Referential(const Referential& other);
        ~Referential();
    public:
        // Copy referential coordinates
        Referential& operator=(const Referential& other);
        // Detach this referential from any parent
        void detach();
        // Attach this referential to given parent
        void attach(Referential& parent);
        // Converts this referential's coordinates to absolute coordinates
        std::pair<Vector, Quaternion> absolute_coordinates() const;
        // Converts this referential's coordinates in another referential
        std::pair<Vector, Quaternion> coordinates_in(const Referential& other) const;
    public:
        Referential* parent = nullptr;
        std::list<Referential*> childrens;
        Vector position; // position in parent's referential (or in absolute referential if parent is null)
        Quaternion orientation; // rotation to apply to transition from a parent referential's (or absolute referential if parent is null) orientation to this referential's
    };
}
