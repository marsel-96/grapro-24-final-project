#include "ituGl/utils/ShaderIncludeReplacer.h"

constexpr std::string_view INCLUDE_IDENTIFIER = "#include";
constexpr std::string_view VERSION_IDENTIFIER = "#version";

constexpr unsigned int OFFSET_INCLUDE = INCLUDE_IDENTIFIER.size() + 2;

// Return the source code of the complete shader
std::string ShaderIncludeReplacer::LoadShaderWithIncludes(const std::string &path) {
    static bool isRecursiveCall = false;

    std::string fullSourceCode;
    std::ifstream file(path);

    if (!file.is_open()) {
        std::cerr << "ERROR: could not open the shader at: " << path << "\n" << std::endl;
        return fullSourceCode;
    }

    std::string lineBuffer;

    while (std::getline(file, lineBuffer)) {
        // Look for the new shader include identifier
        if (lineBuffer.find(INCLUDE_IDENTIFIER) != std::string::npos) {
            // Remove the include identifier, this will cause the path to remain
            lineBuffer.erase(0, OFFSET_INCLUDE);
            lineBuffer.erase(lineBuffer.size() - 1, 1);
            // The include path is relative to the current shader file path
            lineBuffer.insert(0, GetFilePath(path));

            // By using recursion, the new include file can be extracted
            // and inserted at this location in the shader source code
            isRecursiveCall = true;
            fullSourceCode += LoadShaderWithIncludes(lineBuffer);

            // Do not add this line to the shader source code, as the include
            // path would generate a compilation issue in the final source code
            continue;
        }
        if (lineBuffer.find(VERSION_IDENTIFIER) != std::string::npos && isRecursiveCall) {
            continue;
        }
        fullSourceCode += lineBuffer + '\n';
    }

    // Only add the null terminator at the end of the complete file,
    // essentially skipping recursive function calls this way
    if (!isRecursiveCall)
        fullSourceCode += '\0';

    file.close();

    return fullSourceCode;
}

std::string ShaderIncludeReplacer::GetFilePath(const std::string &fullPath) {
    const auto found = fullPath.find_last_of("/\\");
    return fullPath.substr(0, found + 1);
}
