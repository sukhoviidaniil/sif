/***************************************************************
* Author:           <your name>
* Email:            <your email>
* Created:          2026-07-06
*
* License:
*       (c) 2026 <your name>. All rights reserved.
***************************************************************/
#ifndef CYCLE_FINDER_H
#define CYCLE_FINDER_H

#include <vector>
#include "sif/math/Graph.h"

namespace sif::diag {
    /**
     * @brief Finds every cycle in graph.
     *
     * Cycle detection over a DirectedGraph<std::string>, used to find
     * circular include chains in a declaration graph produced by
     * DeclarationGraphBuilder.h. Implemented with Tarjan's strongly
     * connected components algorithm.
     *
     * A cycle is either:
     *   - a strongly connected component with two or more vertices, or
     *   - a single vertex with a self loop (an edge from it to itself).
     *
     * Each returned cycle is the list of vertex ids that belong to it. The
     * order of cycles, and the order of vertices within a cycle, is
     * unspecified but stable for a given graph.
     *
     * @param graph Declaration graph to search for cycles.
     */
    std::vector<std::vector<math::DirectedGraph<std::string>::VertexId>>
    findCycles(const math::DirectedGraph<std::string>& graph);
}

#endif // CYCLE_FINDER_H
