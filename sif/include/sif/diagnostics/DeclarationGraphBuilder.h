/***************************************************************
* Author:           <your name>
* Email:            <your email>
* Created:          2026-07-06
*
* License:
*       (c) 2026 <your name>. All rights reserved.
***************************************************************/
#ifndef DECLARATION_GRAPH_BUILDER_H
#define DECLARATION_GRAPH_BUILDER_H

#include <string>
#include <unordered_map>
#include "sif/math/Graph.h"

namespace sif::diag {
    /**
     * @brief Result of scanning a project folder for classes/structs and includes.
     */
    struct DeclarationGraphResult
    {
        /**
         * @brief Vertex data is the class/struct name; an edge A -> B means
         * A's declaring file includes B's declaring file.
         */
        math::DirectedGraph<std::string> graph;

        /**
         * @brief className -> true if declared with "struct", false if "class".
         */
        std::unordered_map<std::string, bool> isStruct;

        /**
         * @brief className -> path (relative to the scanned folder) of the
         * file where it is declared.
         */
        std::unordered_map<std::string, std::string> declaringFile;
    };

    /**
     * @brief Scans a C/C++ project folder and builds a class/struct dependency graph.
     *
     * Recursively scans folderPath for files with extensions .h .hpp .hh
     * .hxx .cpp .cc .cxx, finds every class/struct definition (forward
     * declarations without a body are ignored), and connects them according
     * to the #include graph between the files that declare them. A cycle
     * in the resulting graph corresponds directly to a circular #include
     * chain.
     *
     * Throws std::runtime_error if folderPath does not exist or is not a
     * directory.
     *
     * @param folderPath Root folder of the project to scan.
     */
    DeclarationGraphResult buildDeclarationGraph(const std::string& folderPath);
}


#endif // DECLARATION_GRAPH_BUILDER_H
