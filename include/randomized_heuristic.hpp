#ifndef RANDOMIZED_HEURISTIC_HPP
#define RANDOMIZED_HEURISTIC_HPP

#include "graph.hpp"
#include <vector>
#include <random>
#include <algorithm>

/**
 * Randomized local search heuristic for maximum clique problem
 * 
 * Algorithm:
 * 1. Start with greedy solution
 * 2. Perform local search with random restarts:
 *    - Try to improve solution by swapping vertices
 *    - Remove 1 vertex, try adding 1-2 vertices
 *    - Accept improvements only
 * 3. Restart from new random initial solution
 * 4. Return best solution across all restarts
 * 
 * Time complexity: O(num_restarts * max_swaps * V²)
 * Space complexity: O(V)
 * 
 * This combines randomization with local improvement for better exploration
 */
class RandomizedHeuristic {
public:
    /**
     * Constructor with configurable parameters
     * @param num_restarts Number of random restarts
     * @param max_swaps Maximum swap attempts per restart
     * @param seed Random seed for reproducibility (0 for random)
     */
    RandomizedHeuristic(int num_restarts = 10, int max_swaps = 1000, unsigned int seed = 0);
    
    /**
     * Find maximum clique using randomized local search
     * @param g Input graph
     * @return Vector of vertex IDs forming the clique
     */
    std::vector<int> find_clique(const Graph& g);
    
private:
    int num_restarts;
    int max_swaps;
    std::mt19937 rng;
    
    /**
     * Perform local search starting from initial solution
     * @param g Input graph
     * @param initial Initial clique
     * @return Improved clique
     */
    std::vector<int> local_search(const Graph& g, std::vector<int> initial);
    
    /**
     * Generate random initial clique
     * @param g Input graph
     * @return Random valid clique
     */
    std::vector<int> random_initial_clique(const Graph& g);
};

#endif // RANDOMIZED_HEURISTIC_HPP
