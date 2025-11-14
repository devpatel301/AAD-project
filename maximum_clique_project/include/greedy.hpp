#ifndef GREEDY_HPP
#define GREEDY_HPP

#include "graph.hpp"
#include <vector>
#include <algorithm>

/**
 * Greedy algorithm for maximum clique problem
 * 
 * Algorithm:
 * 1. Sort vertices by degree in descending order
 * 2. Initialize empty clique
 * 3. For each vertex in sorted order:
 *    - If vertex is connected to all vertices in current clique, add it
 * 4. Return the constructed clique
 * 
 * Time complexity: O(V² + E) - O(V log V) for sorting, O(V²) for clique construction
 * Space complexity: O(V)
 * 
 * Note: This is a fast heuristic but doesn't guarantee optimal solution
 */
class GreedyClique {
public:
    /**
     * Find a maximal clique using greedy approach
     * @param g Input graph
     * @return Vector of vertex IDs forming the clique
     */
    static std::vector<int> find_clique(const Graph& g);
};

#endif // GREEDY_HPP
