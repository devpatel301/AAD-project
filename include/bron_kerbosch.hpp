#ifndef BRON_KERBOSCH_HPP
#define BRON_KERBOSCH_HPP

#include "graph.hpp"
#include <vector>
#include <unordered_set>
#include <algorithm>

/**
 * Basic Bron-Kerbosch algorithm for maximum clique problem
 * 
 * Algorithm:
 * - Recursive backtracking with three sets:
 *   R: current clique being constructed
 *   P: candidate vertices that can extend R
 *   X: vertices already processed (to avoid duplicates)
 * 
 * - Base case: when P and X are empty, R is a maximal clique
 * - Recursive case: for each vertex v in P:
 *   - Add v to R
 *   - Recurse with R∪{v}, P∩N(v), X∩N(v)
 *   - Move v from P to X
 * 
 * Time complexity: O(3^(n/3)) in worst case (exponential)
 * Space complexity: O(n) for recursion depth
 * 
 * This is the basic version without pivoting optimization
 */
class BronKerbosch {
public:
    /**
     * Find maximum clique using basic Bron-Kerbosch algorithm
     * @param g Input graph
     * @return Vector of vertex IDs forming maximum clique
     */
    std::vector<int> find_maximum_clique(const Graph& g);
    
private:
    std::vector<int> max_clique;
    
    /**
     * Recursive Bron-Kerbosch procedure
     * @param R Current clique
     * @param P Candidate set
     * @param X Excluded set
     * @param g Input graph
     */
    void bron_kerbosch(std::unordered_set<int> R, 
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

#endif // BRON_KERBOSCH_HPP
