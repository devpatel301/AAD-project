#ifndef CPU_OPTIMIZED_HPP
#define CPU_OPTIMIZED_HPP

#include "graph.hpp"
#include <vector>
#include <bitset>
#include <algorithm>

/**
 * CPU-optimized maximum clique algorithm
 * 
 * Optimizations over standard Tomita:
 * 1. Use bitsets instead of unordered_set for R, P, X
 *    - Bitwise AND for fast set intersections
 *    - __builtin_popcount for O(1) set size
 * 2. Inline critical functions
 * 3. Cache-friendly memory layout
 * 4. Precompute neighbor bitsets
 * 5. Use array indexing instead of iterators where possible
 * 
 * Time complexity: Same as Tomita, but 5-10x faster in practice
 * Space complexity: O(V²) for bitset adjacency
 * 
 * Note: Limited to graphs with ≤ 1024 vertices due to bitset size
 * For larger graphs, use dynamic bitset or vector<bool>
 */
class CPUOptimized {
public:
    static constexpr int MAX_VERTICES = 1024;
    
    /**
     * Find maximum clique using CPU-optimized algorithm
     * @param g Input graph (must have ≤ MAX_VERTICES vertices)
     * @return Vector of vertex IDs forming maximum clique
     */
    std::vector<int> find_maximum_clique(const Graph& g);
    
private:
    std::vector<int> max_clique;
    std::vector<std::bitset<MAX_VERTICES>> neighbors;
    int n;
    
    /**
     * Optimized Bron-Kerbosch with bitsets
     * @param R Current clique (as bitset)
     * @param P Candidate set (as bitset)
     * @param X Excluded set (as bitset)
     */
    inline void optimized_bk(std::bitset<MAX_VERTICES> R,
                            std::bitset<MAX_VERTICES> P,
                            std::bitset<MAX_VERTICES> X);
    
    /**
     * Choose pivot vertex
     * @param P Candidate set
     * @param X Excluded set
     * @return Pivot vertex ID
     */
    inline int choose_pivot(const std::bitset<MAX_VERTICES>& P,
                           const std::bitset<MAX_VERTICES>& X);
    
    /**
     * Convert bitset to vector of vertex IDs
     * @param bs Bitset
     * @return Vector of set bits
     */
    inline std::vector<int> bitset_to_vector(const std::bitset<MAX_VERTICES>& bs);
};

#endif // CPU_OPTIMIZED_HPP
