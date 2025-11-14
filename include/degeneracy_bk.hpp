#ifndef DEGENERACY_BK_HPP
#define DEGENERACY_BK_HPP

#include "graph.hpp"
#include <vector>
#include <unordered_set>
#include <algorithm>

/**
 * Bron-Kerbosch with degeneracy ordering
 * 
 * Key insight: Use degeneracy ordering to reduce search space
 * 
 * Algorithm:
 * 1. Compute degeneracy ordering of vertices
 * 2. For each vertex v in degeneracy order:
 *    - Let P = neighbors of v that come after v in ordering
 *    - Let X = neighbors of v that come before v in ordering
 *    - Run Tomita (BK with pivoting) on {v} ∪ P with exclusion set X
 * 3. This ensures each maximal clique is found exactly once
 * 
 * Time complexity: O(d * 3^(d/3)) where d is degeneracy
 * Space complexity: O(n)
 * 
 * Optimal for sparse graphs (where d << n)
 * Reference: Eppstein, Löffler, Strash (2010)
 */
class DegeneracyBK {
public:
    /**
     * Find maximum clique using degeneracy ordering + Tomita
     * @param g Input graph
     * @return Vector of vertex IDs forming maximum clique
     */
    std::vector<int> find_maximum_clique(const Graph& g);
    
private:
    std::vector<int> max_clique;
    
    /**
     * Tomita recursive procedure with pivoting
     * @param R Current clique
     * @param P Candidate set
     * @param X Excluded set
     * @param g Graph
     */
    void tomita_with_pivot(std::unordered_set<int> R,
                          std::unordered_set<int> P,
                          std::unordered_set<int> X,
                          const Graph& g);
    
    /**
     * Choose pivot vertex
     * @param P Candidate set
     * @param X Excluded set
     * @param g Graph
     * @return Pivot vertex
     */
    int choose_pivot(const std::unordered_set<int>& P,
                    const std::unordered_set<int>& X,
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

#endif // DEGENERACY_BK_HPP
