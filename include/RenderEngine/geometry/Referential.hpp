#pragma once
#include <RenderEngine/utilities/Macro.hpp>
#include <RenderEngine/geometry/Vector.hpp>
#include <RenderEngine/geometry/Quaternion.hpp>

namespace RenderEngine
{
    class Referential
    {
    public:
        Referential();
        Referential(const Referential& other);
        ~Referential();
    public:
        Referential& operator=(const Referential& other);
    public:
        Referential* parent = nullptr;
        Vector position;
        Quaternion orientation;
    };
}
