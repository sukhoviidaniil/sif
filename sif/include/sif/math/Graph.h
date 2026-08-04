/***************************************************************
* Author:           <your name>
* Email:            <your email>
* Created:          2026-07-06
*
* License:
*       (c) 2026 <your name>. All rights reserved.
***************************************************************/
#ifndef GRAPH_H
#define GRAPH_H

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <stdexcept>
#include <optional>

namespace sif::math {
    /**
     * @brief Generic directed graph storing arbitrary vertex data and edge weights.
     *
     * Every vertex carries a data payload of type T and every edge carries
     * a weight of type W (default double). Backed by an adjacency list.
     * Provides full CRUD operations on vertices and edges, traversal
     * algorithms (BFS / DFS), and degree queries. Vertices are identified
     * by a stable size_t index handed out when the vertex is created.
     */
    template <typename T, typename W = double>
    class DirectedGraph
    {
    public:
        // Type alias for a vertex identifier.
        using VertexId = size_t;

        // ========== Construction / destruction ==========

        DirectedGraph();
        ~DirectedGraph();

        DirectedGraph(const DirectedGraph<T, W>& other) = default;
        DirectedGraph(DirectedGraph<T, W>&& other) noexcept = default;
        DirectedGraph<T, W>& operator=(const DirectedGraph<T, W>& other) = default;
        DirectedGraph<T, W>& operator=(DirectedGraph<T, W>&& other) noexcept = default;

        // ========== Vertex operations ==========

        /**
         * @brief Adds a new vertex carrying the given data and returns its id.
         */
        VertexId addVertex(const T& data);

        /**
         * @brief Removes a vertex and every edge connected to it (incoming or outgoing).
         *
         * Throws std::out_of_range if the vertex does not exist.
         */
        void removeVertex(VertexId id);

        /**
         * @brief Returns true if a vertex with the given id currently exists.
         */
        bool hasVertex(VertexId id) const;

        /**
         * @brief Returns the data stored at the given vertex.
         *
         * Throws std::out_of_range if the vertex does not exist.
         */
        T& getVertexData(VertexId id);
        const T& getVertexData(VertexId id) const;

        /**
         * @brief Convenience operator for getVertexData.
         */
        T& operator[](VertexId id);
        const T& operator[](VertexId id) const;

        /**
         * @brief Overwrites the data stored at the given vertex.
         */
        void setVertexData(VertexId id, const T& data);

        /**
         * @brief Returns how many vertices currently exist in the graph.
         */
        size_t vertexCount() const;

        /**
         * @brief Returns the ids of every vertex currently in the graph.
         */
        std::vector<VertexId> vertexIds() const;

        // ========== Edge operations ==========

        /**
         * @brief Adds a directed edge from -> to with the given weight.
         *
         * If the edge already exists, its weight is updated. Throws
         * std::out_of_range if either endpoint does not exist.
         *
         * @param from Source vertex id.
         * @param to Target vertex id.
         * @param weight Weight assigned to the edge.
         */
        void addEdge(VertexId from, VertexId to, const W& weight = W());

        /**
         * @brief Removes the directed edge from -> to, if present. No-op if it is not.
         */
        void removeEdge(VertexId from, VertexId to);

        /**
         * @brief Returns true if there is a directed edge from -> to.
         */
        bool hasEdge(VertexId from, VertexId to) const;

        /**
         * @brief Returns the weight of the edge from -> to.
         *
         * Throws std::out_of_range if the edge does not exist.
         */
        W getWeight(VertexId from, VertexId to) const;

        /**
         * @brief Updates the weight of an already existing edge.
         *
         * Throws std::out_of_range if the edge does not exist.
         */
        void setWeight(VertexId from, VertexId to, const W& weight);

        /**
         * @brief Returns how many directed edges currently exist in the graph.
         */
        size_t edgeCount() const;

        // ========== Neighborhood queries ==========

        /**
         * @brief Returns the ids of vertices reachable via one outgoing edge from id.
         */
        std::vector<VertexId> outNeighbors(VertexId id) const;

        /**
         * @brief Returns the ids of vertices that have an outgoing edge into id.
         */
        std::vector<VertexId> inNeighbors(VertexId id) const;

        /**
         * @brief Returns the number of outgoing edges from id.
         */
        size_t outDegree(VertexId id) const;

        /**
         * @brief Returns the number of incoming edges into id.
         */
        size_t inDegree(VertexId id) const;

        // ========== Traversal algorithms ==========

        /**
         * @brief Breadth first traversal starting at startId.
         *
         * Returns the order in which vertices were visited. Vertices
         * unreachable from startId are not included.
         */
        std::vector<VertexId> breadthFirstSearch(VertexId startId) const;

        /**
         * @brief Depth first traversal starting at startId.
         *
         * Returns the order in which vertices were visited. Vertices
         * unreachable from startId are not included.
         */
        std::vector<VertexId> depthFirstSearch(VertexId startId) const;

        /**
         * @brief Returns true if there is a directed path from fromId to toId.
         */
        bool isReachable(VertexId fromId, VertexId toId) const;

        // ========== Stream output ==========

        /**
         * @brief Writes the graph to an output stream, one vertex and its
         * outgoing edges per line.
         */
        template <typename U, typename V>
        friend std::ostream& operator<<(std::ostream& os, const DirectedGraph<U, V>& graph);

    private:
        // Adjacency information for a single vertex: outgoing and incoming
        // edges, each mapped to their weight.
        struct AdjacencyEntry
        {
            T data;
            std::unordered_map<VertexId, W> outEdges;
            std::unordered_map<VertexId, W> inEdges;
        };

        std::unordered_map<VertexId, AdjacencyEntry> m_vertices;
        VertexId m_nextId;
        size_t m_edgeCount;

        void checkVertexExists(VertexId id, const char* operationName) const;
        void depthFirstVisit(VertexId current, std::unordered_map<VertexId, bool>& visited,
                              std::vector<VertexId>& order) const;
    };
}

#endif // GRAPH_H
