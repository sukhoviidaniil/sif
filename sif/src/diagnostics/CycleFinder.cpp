/***************************************************************
 * Author:           <your name>
 * Email:            <your email>
 * Created:          2026-07-06
 *
 * License:
 *       (c) 2026 <your name>. All rights reserved.
 ***************************************************************/
/**
 * @brief Implementation of findCycles using Tarjan's strongly connected
 * components algorithm.
 */

#include "sif/diagnostics/CycleFinder.h"

#include <algorithm>
#include <unordered_map>

namespace {

    using VertexId = sif::math::DirectedGraph<std::string>::VertexId;

    /**
     * @brief Mutable state threaded through the recursive Tarjan traversal.
     */
    struct TarjanContext {
        std::unordered_map<VertexId, int> index;
        std::unordered_map<VertexId, int> lowLink;
        std::unordered_map<VertexId, bool> onStack;
        std::vector<VertexId> stack;
        int nextIndex = 0;
        std::vector<std::vector<VertexId>> components;
    };

    void strongConnect(VertexId v, const sif::math::DirectedGraph<std::string>& graph, TarjanContext& ctx) {
        ctx.index[v] = ctx.nextIndex;
        ctx.lowLink[v] = ctx.nextIndex;
        ++ctx.nextIndex;
        ctx.stack.push_back(v);
        ctx.onStack[v] = true;

        for (VertexId neighbor : graph.outNeighbors(v)) {
            if (ctx.index.find(neighbor) == ctx.index.end()) {
                // Neighbor not visited yet: recurse into it.
                strongConnect(neighbor, graph, ctx);
                ctx.lowLink[v] = std::min(ctx.lowLink[v], ctx.lowLink[neighbor]);
            } else if (ctx.onStack[neighbor]) {
                // Neighbor is on the current stack: it is part of the
                // same potential component.
                ctx.lowLink[v] = std::min(ctx.lowLink[v], ctx.index[neighbor]);
            }
        }

        // If v is the root of a strongly connected component, pop it
        // (and everything above it) off the stack.
        if (ctx.lowLink[v] == ctx.index[v]) {
            std::vector<VertexId> component;
            VertexId member;
            do {
                member = ctx.stack.back();
                ctx.stack.pop_back();
                ctx.onStack[member] = false;
                component.push_back(member);
            } while (member != v);

            ctx.components.push_back(component);
        }
    }
} // namespace

namespace sif::diag {
    std::vector<std::vector<VertexId>> findCycles(const math::DirectedGraph<std::string>& graph) {
        TarjanContext ctx;

        for (VertexId v : graph.vertexIds()) {
            if (ctx.index.find(v) == ctx.index.end()) {
                strongConnect(v, graph, ctx);
            }
        }

        std::vector<std::vector<VertexId>> cycles;
        for (auto& component : ctx.components) {
            if (component.size() > 1) {
                cycles.push_back(component);
            } else {
                VertexId only = component.front();
                if (graph.hasEdge(only, only)) {
                    cycles.push_back(component);
                }
            }
        }

        return cycles;
    }
} // namespace sif::diag