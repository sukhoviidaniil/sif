/***************************************************************
* Author:           <your name>
* Email:            <your email>
* Created:          2026-07-06
*
* License:
*       (c) 2026 <your name>. All rights reserved.
***************************************************************/
/**
 * @brief Implementation of the DirectedGraph<T, W> class template.
 *
 * Explicit template instantiations are provided at the bottom of this
 * file for a few commonly used combinations. If you need a different
 * combination of vertex data type / weight type, add an explicit
 * instantiation line.
 */

#include "sif/math/Graph.h"
#include <queue>
#include <string>

namespace sif::math {

    // ========== Construction / destruction ==========
    template <typename T, typename W>
    DirectedGraph<T, W>::DirectedGraph()
        : m_vertices(), m_nextId(0), m_edgeCount(0)
    {
    }

    template <typename T, typename W>
    DirectedGraph<T, W>::~DirectedGraph()
    {
    }

    // ========== Internal helpers ==========

    template <typename T, typename W>
    void DirectedGraph<T, W>::checkVertexExists(VertexId id, const char* operationName) const
    {
        if (m_vertices.find(id) == m_vertices.end())
        {
            throw std::out_of_range(std::string("DirectedGraph::") + operationName +
                                     " - vertex does not exist");
        }
    }

    // ========== Vertex operations ==========

    template <typename T, typename W>
    typename DirectedGraph<T, W>::VertexId DirectedGraph<T, W>::addVertex(const T& data)
    {
        VertexId newId = m_nextId;
        ++m_nextId;

        AdjacencyEntry entry;
        entry.data = data;
        m_vertices.emplace(newId, std::move(entry));

        return newId;
    }

    template <typename T, typename W>
    void DirectedGraph<T, W>::removeVertex(VertexId id)
    {
        checkVertexExists(id, "removeVertex");

        AdjacencyEntry& entry = m_vertices.at(id);

        // Remove every outgoing edge from id, updating the target's inEdges.
        for (const auto& outEdge : entry.outEdges)
        {
            VertexId target = outEdge.first;
            m_vertices.at(target).inEdges.erase(id);
            --m_edgeCount;
        }

        // Remove every incoming edge into id, updating the source's outEdges.
        for (const auto& inEdge : entry.inEdges)
        {
            VertexId source = inEdge.first;
            m_vertices.at(source).outEdges.erase(id);
            --m_edgeCount;
        }

        m_vertices.erase(id);
    }

    template <typename T, typename W>
    bool DirectedGraph<T, W>::hasVertex(VertexId id) const
    {
        return m_vertices.find(id) != m_vertices.end();
    }

    template <typename T, typename W>
    T& DirectedGraph<T, W>::getVertexData(VertexId id)
    {
        checkVertexExists(id, "getVertexData");
        return m_vertices.at(id).data;
    }

    template <typename T, typename W>
    const T& DirectedGraph<T, W>::getVertexData(VertexId id) const
    {
        checkVertexExists(id, "getVertexData");
        return m_vertices.at(id).data;
    }

    template <typename T, typename W>
    T& DirectedGraph<T, W>::operator[](VertexId id)
    {
        return getVertexData(id);
    }

    template <typename T, typename W>
    const T& DirectedGraph<T, W>::operator[](VertexId id) const
    {
        return getVertexData(id);
    }

    template <typename T, typename W>
    void DirectedGraph<T, W>::setVertexData(VertexId id, const T& data)
    {
        checkVertexExists(id, "setVertexData");
        m_vertices.at(id).data = data;
    }

    template <typename T, typename W>
    size_t DirectedGraph<T, W>::vertexCount() const
    {
        return m_vertices.size();
    }

    template <typename T, typename W>
    std::vector<typename DirectedGraph<T, W>::VertexId> DirectedGraph<T, W>::vertexIds() const
    {
        std::vector<VertexId> result;
        result.reserve(m_vertices.size());
        for (const auto& entry : m_vertices)
        {
            result.push_back(entry.first);
        }
        return result;
    }

    // ========== Edge operations ==========

    template <typename T, typename W>
    void DirectedGraph<T, W>::addEdge(VertexId from, VertexId to, const W& weight)
    {
        checkVertexExists(from, "addEdge");
        checkVertexExists(to, "addEdge");

        AdjacencyEntry& fromEntry = m_vertices.at(from);
        bool isNewEdge = (fromEntry.outEdges.find(to) == fromEntry.outEdges.end());

        fromEntry.outEdges[to] = weight;
        m_vertices.at(to).inEdges[from] = weight;

        if (isNewEdge)
        {
            ++m_edgeCount;
        }
    }

    template <typename T, typename W>
    void DirectedGraph<T, W>::removeEdge(VertexId from, VertexId to)
    {
        auto fromIt = m_vertices.find(from);
        if (fromIt == m_vertices.end())
        {
            return;
        }

        auto& outEdges = fromIt->second.outEdges;
        auto edgeIt = outEdges.find(to);
        if (edgeIt == outEdges.end())
        {
            return;
        }

        outEdges.erase(edgeIt);
        m_vertices.at(to).inEdges.erase(from);
        --m_edgeCount;
    }

    template <typename T, typename W>
    bool DirectedGraph<T, W>::hasEdge(VertexId from, VertexId to) const
    {
        auto fromIt = m_vertices.find(from);
        if (fromIt == m_vertices.end())
        {
            return false;
        }
        return fromIt->second.outEdges.find(to) != fromIt->second.outEdges.end();
    }

    template <typename T, typename W>
    W DirectedGraph<T, W>::getWeight(VertexId from, VertexId to) const
    {
        checkVertexExists(from, "getWeight");
        const auto& outEdges = m_vertices.at(from).outEdges;
        auto edgeIt = outEdges.find(to);
        if (edgeIt == outEdges.end())
        {
            throw std::out_of_range("DirectedGraph::getWeight - edge does not exist");
        }
        return edgeIt->second;
    }

    template <typename T, typename W>
    void DirectedGraph<T, W>::setWeight(VertexId from, VertexId to, const W& weight)
    {
        checkVertexExists(from, "setWeight");
        auto& outEdges = m_vertices.at(from).outEdges;
        if (outEdges.find(to) == outEdges.end())
        {
            throw std::out_of_range("DirectedGraph::setWeight - edge does not exist");
        }
        outEdges[to] = weight;
        m_vertices.at(to).inEdges[from] = weight;
    }

    template <typename T, typename W>
    size_t DirectedGraph<T, W>::edgeCount() const
    {
        return m_edgeCount;
    }

    // ========== Neighborhood queries ==========

    template <typename T, typename W>
    std::vector<typename DirectedGraph<T, W>::VertexId> DirectedGraph<T, W>::outNeighbors(VertexId id) const
    {
        checkVertexExists(id, "outNeighbors");
        std::vector<VertexId> result;
        const auto& outEdges = m_vertices.at(id).outEdges;
        result.reserve(outEdges.size());
        for (const auto& edge : outEdges)
        {
            result.push_back(edge.first);
        }
        return result;
    }

    template <typename T, typename W>
    std::vector<typename DirectedGraph<T, W>::VertexId> DirectedGraph<T, W>::inNeighbors(VertexId id) const
    {
        checkVertexExists(id, "inNeighbors");
        std::vector<VertexId> result;
        const auto& inEdges = m_vertices.at(id).inEdges;
        result.reserve(inEdges.size());
        for (const auto& edge : inEdges)
        {
            result.push_back(edge.first);
        }
        return result;
    }

    template <typename T, typename W>
    size_t DirectedGraph<T, W>::outDegree(VertexId id) const
    {
        checkVertexExists(id, "outDegree");
        return m_vertices.at(id).outEdges.size();
    }

    template <typename T, typename W>
    size_t DirectedGraph<T, W>::inDegree(VertexId id) const
    {
        checkVertexExists(id, "inDegree");
        return m_vertices.at(id).inEdges.size();
    }

    // ========== Traversal algorithms ==========

    template <typename T, typename W>
    std::vector<typename DirectedGraph<T, W>::VertexId> DirectedGraph<T, W>::breadthFirstSearch(VertexId startId) const
    {
        checkVertexExists(startId, "breadthFirstSearch");

        std::vector<VertexId> order;
        std::unordered_map<VertexId, bool> visited;
        std::queue<VertexId> toVisit;

        visited[startId] = true;
        toVisit.push(startId);

        while (!toVisit.empty())
        {
            VertexId current = toVisit.front();
            toVisit.pop();
            order.push_back(current);

            for (const auto& edge : m_vertices.at(current).outEdges)
            {
                VertexId neighbor = edge.first;
                if (!visited[neighbor])
                {
                    visited[neighbor] = true;
                    toVisit.push(neighbor);
                }
            }
        }

        return order;
    }

    template <typename T, typename W>
    void DirectedGraph<T, W>::depthFirstVisit(VertexId current, std::unordered_map<VertexId, bool>& visited,
                                               std::vector<VertexId>& order) const
    {
        visited[current] = true;
        order.push_back(current);

        for (const auto& edge : m_vertices.at(current).outEdges)
        {
            VertexId neighbor = edge.first;
            if (!visited[neighbor])
            {
                depthFirstVisit(neighbor, visited, order);
            }
        }
    }

    template <typename T, typename W>
    std::vector<typename DirectedGraph<T, W>::VertexId> DirectedGraph<T, W>::depthFirstSearch(VertexId startId) const
    {
        checkVertexExists(startId, "depthFirstSearch");

        std::vector<VertexId> order;
        std::unordered_map<VertexId, bool> visited;
        depthFirstVisit(startId, visited, order);
        return order;
    }

    template <typename T, typename W>
    bool DirectedGraph<T, W>::isReachable(VertexId fromId, VertexId toId) const
    {
        checkVertexExists(fromId, "isReachable");
        checkVertexExists(toId, "isReachable");

        std::vector<VertexId> reachable = breadthFirstSearch(fromId);
        for (VertexId id : reachable)
        {
            if (id == toId)
            {
                return true;
            }
        }
        return false;
    }

    // ========== Free functions ==========

    template <typename U, typename V>
    std::ostream& operator<<(std::ostream& os, const DirectedGraph<U, V>& graph)
    {
        for (const auto& entry : graph.m_vertices)
        {
            typename DirectedGraph<U, V>::VertexId id = entry.first;
            os << "Vertex " << id << " (" << entry.second.data << ") -> [";

            bool first = true;
            for (const auto& edge : entry.second.outEdges)
            {
                if (!first)
                {
                    os << ", ";
                }
                os << edge.first << " (w=" << edge.second << ")";
                first = false;
            }
            os << "]\n";
        }
        return os;
    }

    // ========== Explicit template instantiations ==========
    // Add a line here for every (data type, weight type) combination you need.

    template class DirectedGraph<int, double>;
    template class DirectedGraph<std::string, double>;
    template class DirectedGraph<int, int>;

    template std::ostream& operator<<(std::ostream&, const DirectedGraph<int, double>&);
    template std::ostream& operator<<(std::ostream&, const DirectedGraph<std::string, double>&);
    template std::ostream& operator<<(std::ostream&, const DirectedGraph<int, int>&);
}