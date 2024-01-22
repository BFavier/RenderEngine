#pragma once
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>
#include <math.h>

#include "Macro.hpp"

namespace RenderEngine
{

    class Utilities
    {
    public:
        static unsigned int log2(unsigned int x);
        static unsigned int pow2(unsigned int power);
        ///< Returns the extension of a file path (or an empty string if there is none)
        static std::string extension(const std::string& file_name);
        ///< Returns a string in full upper case
        static std::string to_upper(const std::string& str);
        ///< Simplify a Linux styled path (removes "/./", "//" and "/item/../" entries in a path)
        static std::string simplify_path(const std::string& path);
        ///< Replace all matching substrings 'searched' by 'replacement' in place. Returns the number of replacements.
        static unsigned int replace_substrings(std::string& str, const std::string& searched, const std::string& replacement);
    };
}
