#pragma once

#include <sstream>
#include <string>
#include <unordered_set>

namespace S2Plugin
{
    class CPPSyntaxHighlighter;

    class CPPGenerator
    {
      public:
        void generate(const std::string& typeName, CPPSyntaxHighlighter* highlighter);
        std::string result() const;

      private:
        std::stringstream mSS;
        std::unordered_set<std::string> mGeneratedTypes; // so we don't dump the same one twice
    };
} // namespace S2Plugin
