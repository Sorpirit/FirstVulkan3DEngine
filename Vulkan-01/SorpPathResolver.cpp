#include "SorpPathResolver.h"

#include <Windows.h>

namespace sorp_v
{
    const std::string SorpPathResolver::CONTENT = "Content";
    
    SorpPathResolver::SorpPathResolver() : _content(getContentPath())
    {
    }

    std::string SorpPathResolver::resolve(const std::string& path) const
    {
        return resolve(std::filesystem::path(path));
    }

    std::string SorpPathResolver::resolve(const std::filesystem::path& path) const
    {
        return (_content / path).string();
    }

    std::string SorpPathResolver::getContentPath()
    {
        auto root = std::filesystem::current_path();
        auto diskRoot = root.root_path();
        
        auto content = root / CONTENT;
        
        while(!exists(content) && !equivalent(root, diskRoot))
        {
            root = root.parent_path();
            content = root / CONTENT;
        }

        if(equivalent(root, diskRoot))
            throw std::runtime_error("Content directory not found! Make sure there is a path from executable directory to Content directory.");
        
        return content.string();
    }
}
