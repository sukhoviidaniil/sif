/***************************************************************
* Author:           <your name>
* Email:            <your email>
* Created:          2026-07-06
*
* License:
*       (c) 2026 <your name>. All rights reserved.
***************************************************************/
#ifndef PLANT_UML_EXPORTER_H
#define PLANT_UML_EXPORTER_H

#include <string>
#include <vector>
#include "sif/math/Graph.h"
#include "DeclarationGraphBuilder.h"

namespace sif::diag {
    /**
     * @brief Writes a PlantUML class diagram for a declaration graph.
     *
     * Every vertex becomes a class/struct entry; edges that lie inside the
     * same cycle (as reported by CycleFinder.h) are drawn in a color
     * unique to that cycle, every other edge is drawn in black. Creates
     * outputFolder (recursively) if it does not exist yet.
     *
     * @param graph Declaration graph to render.
     * @param metadata Extra per-vertex information (struct/class, file).
     * @param cycles Cycles previously found by findCycles.
     * @param outputFolder Folder the diagram file is written into.
     * @param fileName Name of the generated file.
     *
     * @return Full path of the file that was written.
     */
    std::string exportToPlantUml(
        const math::DirectedGraph<std::string>& graph,
        const DeclarationGraphResult& metadata,
        const std::vector<std::vector<math::DirectedGraph<std::string>::VertexId>>& cycles,
        const std::string& outputFolder,
        const std::string& fileName = "declaration_graph.puml");
}

#endif // PLANT_UML_EXPORTER_H
