#pragma once

#include <string>
#include <filesystem>

namespace sorp_v
{
    class SorpPathResolver
    {
    public:
        SorpPathResolver();
    
        std::string resolve(const std::string& path) const;
        std::string resolve(const std::filesystem::path& path) const;

        static const std::string CONTENT;
    
    private:
        std::filesystem::path _content;
        
        std::string getContentPath();
    };
    
}
