/***************************************************************
* Author:           <your name>
* Email:            <your email>
* Created:          2026-07-06
*
* License:
*       (c) 2026 <your name>. All rights reserved.
***************************************************************/
/**
 * @brief Implementation of exportToPlantUml.
 */

#include "sif/diagnostics/PlantUmlExporter.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <unordered_map>

namespace fs = std::filesystem;

namespace
{
    using VertexId = sif::math::DirectedGraph<std::string>::VertexId;

    /**
     * @brief A palette of visually distinct colors, one per cycle.
     *
     * If there are more cycles than colors, colors are reused (wrap around).
     */
    const std::vector<std::string>& colorPalette()
    {
        static const std::vector<std::string> palette =
        {
            "#E6194B", "#3CB44B", "#4363D8", "#F58231", "#911EB4",
            "#46F0F0", "#F032E6", "#BCF60C", "#008080", "#9A6324",
            "#800000", "#FABEBE", "#E6BEFF", "#AAFFC3", "#808000"
        };
        return palette;
    }

    /**
     * @brief Vertex ids are unique, so using them for the PlantUML alias
     * avoids any name clash even if two classes share a display name.
     */
    std::string aliasFor(VertexId id)
    {
        return "N" + std::to_string(id);
    }
}
namespace sif::diag {

    std::string exportToPlantUml(
        const math::DirectedGraph<std::string>& graph,
        const DeclarationGraphResult& metadata,
        const std::vector<std::vector<VertexId>>& cycles,
        const std::string& outputFolder,
        const std::string& fileName)
    {
        fs::create_directories(outputFolder);

        fs::path outputPath = fs::path(outputFolder) / fileName;
        std::ofstream out(outputPath);
        if (!out.is_open())
        {
            throw std::runtime_error("exportToPlantUml - cannot open output file: " + outputPath.string());
        }

        // Map every vertex that belongs to a cycle to that cycle's index.
        std::unordered_map<VertexId, size_t> vertexToCycleIndex;
        for (size_t cycleIndex = 0; cycleIndex < cycles.size(); ++cycleIndex)
        {
            for (VertexId vertex : cycles[cycleIndex])
            {
                vertexToCycleIndex[vertex] = cycleIndex;
            }
        }

        out << "@startuml\n";
        out << "' Auto-generated declaration dependency diagram.\n";
        out << "' Each include cycle has its own arrow color; black = no cycle.\n\n";

        // ========== Vertex declarations ==========
        for (VertexId vertex : graph.vertexIds())
        {
            const std::string& name = graph[vertex];
            bool isStruct = metadata.isStruct.count(name) > 0 && metadata.isStruct.at(name);
            std::string alias = aliasFor(vertex);

            out << (isStruct ? "struct " : "class ") << "\"" << name << "\" as " << alias;

            auto cycleIt = vertexToCycleIndex.find(vertex);
            if (cycleIt != vertexToCycleIndex.end())
            {
                out << " " << colorPalette()[cycleIt->second % colorPalette().size()];
            }
            out << "\n";
        }

        out << "\n";

        // ========== Edge declarations ==========
        for (VertexId from : graph.vertexIds())
        {
            std::string fromAlias = aliasFor(from);
            auto fromCycleIt = vertexToCycleIndex.find(from);

            for (VertexId to : graph.outNeighbors(from))
            {
                std::string toAlias = aliasFor(to);
                auto toCycleIt = vertexToCycleIndex.find(to);

                bool sameCycle = fromCycleIt != vertexToCycleIndex.end() &&
                                  toCycleIt != vertexToCycleIndex.end() &&
                                  fromCycleIt->second == toCycleIt->second;

                if (sameCycle)
                {
                    const std::string& color = colorPalette()[fromCycleIt->second % colorPalette().size()];
                    out << fromAlias << " -[" << color << "]-> " << toAlias << "\n";
                }
                else
                {
                    out << fromAlias << " -[#000000]-> " << toAlias << "\n";
                }
            }
        }

        // ========== Legend describing which classes form each cycle ==========
        if (!cycles.empty())
        {
            out << "\nlegend right\n";
            out << "Include cycles found: " << cycles.size() << "\n";
            for (size_t i = 0; i < cycles.size(); ++i)
            {
                out << "Cycle " << (i + 1) << " (" << colorPalette()[i % colorPalette().size()] << "): ";
                for (size_t j = 0; j < cycles[i].size(); ++j)
                {
                    out << graph[cycles[i][j]];
                    if (j + 1 < cycles[i].size())
                    {
                        out << ", ";
                    }
                }
                out << "\n";
            }
            out << "endlegend\n";
        }

        out << "@enduml\n";
        out.close();

        return outputPath.string();
    }
}