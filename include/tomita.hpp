#ifndef TOMITA_HPP
#define TOMITA_HPP

#include "graph.hpp"
#include <vector>
#include <unordered_set>
#include <algorithm>

/**
 * Tomita algorithm: Bron-Kerbosch with pivoting
 * 
 * Key optimization over basic Bron-Kerbosch:
 * - Choose a pivot vertex u from P ∪ X
 * - Only recurse on vertices in P \ N(u)
 * - Pivot selection: choose u that maximizes |P ∩ N(u)|
 * - This minimizes the number of recursive calls
 * 
 * Algorithm by Tomita, Tanaka, and Takahashi (2006)
 * 
 * Time complexity: O(3^(n/3)) worst case, but much faster in practice
 * Space complexity: O(n) for recursion depth
 * 
 * Typically 10-100x faster than basic Bron-Kerbosch on real graphs
 */
class TomitaAlgorithm {
public:
    /**
     * Find maximum clique using Tomita algorithm (BK with pivoting)
     * @param g Input graph
     * @return Vector of vertex IDs forming maximum clique
     */
    std::vector<int> find_maximum_clique(const Graph& g);
    
private:
    std::vector<int> max_clique;
    
    /**
     * Choose pivot vertex that maximizes |P ∩ N(pivot)|
     * @param P Candidate set
     * @param X Excluded set
     * @param g Graph
     * @return Pivot vertex
     */
    int choose_pivot(const std::unordered_set<int>& P,
                    const std::unordered_set<int>& X,
                    const Graph& g);
    
    /**
     * Recursive Tomita procedure with pivoting
     * @param R Current clique
     * @param P Candidate set
     * @param X Excluded set
     * @param g Graph
     */
    void tomita_recursive(std::unordered_set<int> R,
                         std::unordered_set<int> P,
                         std::unordered_set<int> X,
                         const Graph& g);
    
    /**
     * Compute intersection of set and vertex neighbors
     * @param s Input set
     * @param v Vertex
     * @param g Graph
     * @return s ∩ N(v)
     */
    std::unordered_set<int> intersect_with_neighbors(
        const std::unordered_set<int>& s, int v, const Graph& g);
};

#endif // TOMITA_HPP
