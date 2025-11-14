#ifndef OSTERGARD_HPP
#define OSTERGARD_HPP

#include "graph.hpp"
#include <vector>
#include <algorithm>

/**
 * Östergård's algorithm for maximum clique
 * 
 * Branch-and-bound algorithm with vertex coloring for upper bounds
 * 
 * Key ideas:
 * 1. Order vertices by decreasing degree
 * 2. Use greedy graph coloring to compute upper bound on clique size
 * 3. Prune branches when: current_size + upper_bound ≤ best_known_size
 * 4. Branch on vertices in order, maintaining candidate set
 * 
 * Coloring bound: If vertices can be colored with k colors, 
 * then maximum clique size in that subgraph is at most k
 * (because clique vertices must all have different colors)
 * 
 * Time complexity: Exponential, but with effective pruning
 * Space complexity: O(n) for recursion
 * 
 * Reference: Östergård (2002) "A fast algorithm for the maximum clique problem"
 */
class OstergardAlgorithm {
public:
    /**
     * Find maximum clique using Östergård's algorithm
     * @param g Input graph
     * @return Vector of vertex IDs forming maximum clique
     */
    std::vector<int> find_maximum_clique(const Graph& g);
    
private:
    std::vector<int> max_clique;
    
    /**
     * Compute upper bound on clique size using greedy coloring
     * Colors vertices greedily and returns number of colors used
     * 
     * @param candidates Candidate vertices
     * @param g Graph
     * @return Upper bound (number of colors)
     */
    int color_bound(const std::vector<int>& candidates, const Graph& g);
    
    /**
     * Branch-and-bound recursive procedure
     * @param current Current clique being constructed
     * @param candidates Remaining candidate vertices
     * @param g Graph
     */
    void branch_and_bound(std::vector<int> current,
                         std::vector<int> candidates,
                         const Graph& g);
};

#endif // OSTERGARD_HPP
