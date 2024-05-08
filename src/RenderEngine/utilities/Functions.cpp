#include <RenderEngine/utilities/Functions.hpp>
using namespace RenderEngine;

unsigned int Utilities::log2(unsigned int x)
{
    unsigned int k = 0;
    while(x != 0)
    {
        x = x >> 1;
        k++;
    }
    return k;
}

unsigned int Utilities::pow2(unsigned int power)
{
    return (1 << power);
}

bool Utilities::ends_with(const std::string& string, const std::string& suffix)
{
    if (string.length() >= suffix.length())
    {
        return (0 == string.compare(string.length() - suffix.length(), suffix.length(), suffix));
    }
    else
    {
        return false;
    }
}

std::string Utilities::extension(const std::string& file_name)
{
    size_t n = file_name.find_last_of(".", std::string::npos);
    return file_name.substr(n+1);
}

std::string Utilities::to_upper(const std::string& str)
{
    std::string upper;
    for (const char c : str)
    {
        upper.push_back(std::toupper(c));
    }
    return upper;
}

//Simplify a path
std::string Utilities::simplify_path(const std::string& path)
{
    //Extract the directories into a list of string
    std::list<std::string> directories;
    unsigned int n = path.length();
    size_t j,i=0;//i is the first position after the previous "/", j is the position of the next "/".
    while (true)
    {
        //Find the next "/" character in the path
        j = path.find("/",i);
        if (j == std::string::npos)
            j = n;
        //extract the item between the two "/"
        std::string item;
        if (j > i)
            item = path.substr(i,j-i);
        else//If there was two consecutive "/" characters, or the first character was a "/"
            item = "";
        //increments the tail position
        i = j+1;
        //Process the item read
        if (item == "." || item == "")
        {
            //Do nothing
        }
        else if((item == "..") && !(directories.empty()))
            directories.pop_back();
        else
            directories.push_back(item);
        //Stop if we reached the end
        if (i >= n)
        {
            break;
        }
    }
    //pop the directories from the list and create a simplified path
    std::string result = "";
    while (directories.size() > 0)
    {
        result += directories.front();
        directories.pop_front();
        if (directories.size() > 0)
            result += "/";
    }
    return result;
}

unsigned int Utilities::replace_substrings(std::string& str, const std::string& searched, const std::string& replacement)
{
    unsigned int k = 0;
    if(searched.empty())
        return 0;
    size_t start_pos = str.find(searched, 0);
    while(start_pos != std::string::npos)
    {
        str.replace(start_pos, searched.length(), replacement);
        start_pos += replacement.length();
        start_pos = str.find(searched, start_pos);
        k ++;
    }
    return k;
}
