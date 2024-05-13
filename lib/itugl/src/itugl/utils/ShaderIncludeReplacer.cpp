#include "ituGl/utils/ShaderIncludeReplacer.h"

#include <set>

constexpr std::string_view INCLUDE_IDENTIFIER = "#include";
constexpr std::string_view VERSION_IDENTIFIER = "#version";

// INCLUDE_IDENTIFIER size + 2 for the space and the quotation mark
constexpr unsigned int OFFSET_INCLUDE = INCLUDE_IDENTIFIER.size() + 2;

struct RecursiveData {
    std::set<std::string> includes;
    unsigned char depth = 0;
};


std::string ShaderIncludeReplacer::LoadShaderWithIncludes(const std::string &path, RecursiveData &data) { // NOLINT(*-no-recursion)
    std::string code;
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "ERROR: could not open the shader at: " << path << "\n" << std::endl;
        return code;
    }

    std::string lineBuffer;

    while (std::getline(file, lineBuffer)) {
        // Look for the new shader include identifier
        if (lineBuffer.find(INCLUDE_IDENTIFIER) != std::string::npos) {

            // Include the same file only once
            lineBuffer.erase(0, OFFSET_INCLUDE);
            lineBuffer.pop_back();

            if (!data.includes.contains(lineBuffer)) {

                data.includes.insert(lineBuffer);
                data.depth++;

                lineBuffer.insert(0, GetFilePath(path));
                code += LoadShaderWithIncludes(lineBuffer, data);
            }

            continue;
        }
        // Remopve the version identifier from the included files
        if (lineBuffer.find(VERSION_IDENTIFIER) != std::string::npos && data.depth > 0) {
            continue;
        }
        code += lineBuffer + '\n';
    }

    file.close();

    return code;
}

// Return the source code of the complete shader
std::string ShaderIncludeReplacer::LoadShaderWithIncludes(const std::string &path) {

    RecursiveData recursiveData;

    // Add the null terminator at the end of the file
    std::string sourceCode = LoadShaderWithIncludes(path, recursiveData) + '\0';

    return sourceCode;
}

std::string ShaderIncludeReplacer::GetFilePath(const std::string &fullPath) {
    const auto found = fullPath.find_last_of("/\\");
    return fullPath.substr(0, found + 1);
}
