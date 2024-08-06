#pragma once
#include <typeinfo>
#include <string>

namespace RenderEnginer
{
    class Drawable
    {
    public:
        // Returns a string containing the typename of the drawable (most specialized type in case of inheritance)
        std::string type_name() const {return typeid(*this).name();};
    };
}