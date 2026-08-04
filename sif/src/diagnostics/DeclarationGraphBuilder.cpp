/***************************************************************
* Author:           <your name>
* Email:            <your email>
* Created:          2026-07-06
*
* License:
*       (c) 2026 <your name>. All rights reserved.
***************************************************************/
/**
 * @brief Implementation of buildDeclarationGraph.
 *
 * Uses lightweight regex based text scanning rather than a full C++
 * parser, which is enough to detect #include relationships and top
 * level class/struct definitions.
 */

#include "sif/diagnostics/DeclarationGraphBuilder.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <regex>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;

namespace
{
    /**
     * @brief File extensions that are considered part of a C/C++ project.
     */
    bool hasSourceExtension(const fs::path& path)
    {
        static const std::vector<std::string> extensions =
            { ".h", ".hpp", ".hh", ".hxx", ".cpp", ".cc", ".cxx" };

        std::string ext = path.extension().string();
        for (const auto& candidate : extensions)
        {
            if (ext == candidate)
            {
                return true;
            }
        }
        return false;
    }

    std::string readWholeFile(const fs::path& path)
    {
        std::ifstream file(path);
        if (!file.is_open())
        {
            throw std::runtime_error("buildDeclarationGraph - cannot open file: " + path.string());
        }
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    std::string normalizeSeparators(const std::string& text)
    {
        std::string result = text;
        for (char& c : result)
        {
            if (c == '\\')
            {
                c = '/';
            }
        }
        return result;
    }

    /**
     * @brief Returns true if fullPath ends with suffix, aligned on a path
     * separator boundary (so "FooBar.h" does not match suffix "Bar.h").
     */
    bool endsWithPathSuffix(const std::string& fullPath, const std::string& suffix)
    {
        std::string normalizedFull = normalizeSeparators(fullPath);
        std::string normalizedSuffix = normalizeSeparators(suffix);

        if (normalizedSuffix.size() > normalizedFull.size())
        {
            return false;
        }

        size_t startPos = normalizedFull.size() - normalizedSuffix.size();
        if (startPos > 0 && normalizedFull[startPos - 1] != '/')
        {
            return false;
        }

        return normalizedFull.compare(startPos, normalizedSuffix.size(), normalizedSuffix) == 0;
    }
}

namespace sif::diag {
    DeclarationGraphResult buildDeclarationGraph(const std::string& folderPath)
    {
        fs::path root(folderPath);
        if (!fs::exists(root) || !fs::is_directory(root))
        {
            throw std::runtime_error("buildDeclarationGraph - folder does not exist: " + folderPath);
        }

        // ========== Step 1: collect every relevant source file and its text ==========

        std::vector<fs::path> sourceFiles;
        std::unordered_map<std::string, std::string> fileContents; // key: absolute path

        for (const auto& entry : fs::recursive_directory_iterator(root))
        {
            if (entry.is_regular_file() && hasSourceExtension(entry.path()))
            {
                sourceFiles.push_back(entry.path());
                fileContents[entry.path().string()] = readWholeFile(entry.path());
            }
        }

        // ========== Step 2: extract #include directives per file ==========

        std::regex includeRegex(R"(#\s*include\s*[<\"]([^\">]+)[>\"])");
        std::unordered_map<std::string, std::vector<std::string>> fileIncludes; // file -> resolved included files

        for (const auto& file : sourceFiles)
        {
            const std::string& content = fileContents.at(file.string());
            auto begin = std::sregex_iterator(content.begin(), content.end(), includeRegex);
            auto end = std::sregex_iterator();

            for (auto it = begin; it != end; ++it)
            {
                std::string includedName = (*it)[1].str();

                for (const auto& candidate : sourceFiles)
                {
                    if (endsWithPathSuffix(candidate.string(), includedName))
                    {
                        fileIncludes[file.string()].push_back(candidate.string());
                        break; // first matching file wins
                    }
                }
            }
        }

        // ========== Step 3: extract class/struct definitions per file ==========
        // Requires a following '{' so pure forward declarations ("class Foo;")
        // are ignored, since they do not "declare" the type, only mention it.
        std::regex declRegex(R"(\b(class|struct)\s+([A-Za-z_]\w*)\s*(?:final\s*)?(?:[:][^\{;]*)?\{)");

        DeclarationGraphResult result;
        std::unordered_map<std::string, math::DirectedGraph<std::string>::VertexId> nameToVertex;
        std::unordered_map<std::string, std::vector<std::string>> classesInFile; // file -> class names

        for (const auto& file : sourceFiles)
        {
            const std::string& content = fileContents.at(file.string());
            auto begin = std::sregex_iterator(content.begin(), content.end(), declRegex);
            auto end = std::sregex_iterator();

            for (auto it = begin; it != end; ++it)
            {
                std::string keyword = (*it)[1].str();
                std::string name = (*it)[2].str();

                if (nameToVertex.find(name) == nameToVertex.end())
                {
                    auto vertexId = result.graph.addVertex(name);
                    nameToVertex[name] = vertexId;
                    result.isStruct[name] = (keyword == "struct");
                    result.declaringFile[name] = fs::relative(file, root).string();
                }

                classesInFile[file.string()].push_back(name);
            }
        }

        // ========== Step 4: build edges from the include graph ==========
        // For every include "file -> includedFile", connect every class
        // declared in file to every class declared in includedFile.

        for (const auto& file : sourceFiles)
        {
            std::string filePath = file.string();

            auto classesIt = classesInFile.find(filePath);
            if (classesIt == classesInFile.end())
            {
                continue; // no classes/structs declared in this file
            }

            auto includesIt = fileIncludes.find(filePath);
            if (includesIt == fileIncludes.end())
            {
                continue; // no local includes from this file
            }

            for (const std::string& sourceClass : classesIt->second)
            {
                auto sourceVertex = nameToVertex.at(sourceClass);

                for (const std::string& includedFile : includesIt->second)
                {
                    auto targetClassesIt = classesInFile.find(includedFile);
                    if (targetClassesIt == classesInFile.end())
                    {
                        continue; // included file declares no classes/structs
                    }

                    for (const std::string& targetClass : targetClassesIt->second)
                    {
                        if (targetClass == sourceClass)
                        {
                            continue; // skip accidental self edges
                        }
                        auto targetVertex = nameToVertex.at(targetClass);
                        result.graph.addEdge(sourceVertex, targetVertex);
                    }
                }
            }
        }

        return result;
    }
}