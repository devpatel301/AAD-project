#ifndef SIMULATED_ANNEALING_HPP
#define SIMULATED_ANNEALING_HPP

#include "graph.hpp"
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

/**
 * Simulated Annealing metaheuristic for maximum clique problem
 * 
 * Algorithm:
 * 1. Start with initial solution (greedy clique)
 * 2. Iteratively generate neighbor solutions by:
 *    - Adding a vertex that can extend the clique
 *    - Removing a vertex from the clique
 *    - Swapping a vertex (remove one, add another)
 * 3. Accept better solutions always
 * 4. Accept worse solutions with probability exp(-ΔE/T)
 * 5. Gradually decrease temperature T
 * 6. Return best solution found
 * 
 * Time complexity: O(max_iterations * V²) in worst case
 * Space complexity: O(V)
 * 
 * Parameters:
 * - initial_temp: Starting temperature (default 100.0)
 * - cooling_rate: Temperature multiplier per iteration (default 0.995)
 * - max_iterations: Number of iterations (default 100000)
 */
class SimulatedAnnealing {
public:
    /**
     * Constructor with configurable parameters
     * @param initial_temp Starting temperature
     * @param cooling_rate Cooling rate (0 < rate < 1)
     * @param max_iterations Maximum number of iterations
     * @param seed Random seed for reproducibility (0 for random)
     */
    SimulatedAnnealing(double initial_temp = 100.0, 
                      double cooling_rate = 0.995, 
                      int max_iterations = 100000,
                      unsigned int seed = 0);
    
    /**
     * Find maximum clique using simulated annealing
     * @param g Input graph
     * @return Vector of vertex IDs forming the clique
     */
    std::vector<int> find_clique(const Graph& g);
    
private:
    double temperature;
    double initial_temperature;
    double cooling_rate;
    int max_iterations;
    std::mt19937 rng;
    
    /**
     * Generate a neighbor solution by random modification
     * @param current Current clique
     * @param g Input graph
     * @return New candidate clique
     */
    std::vector<int> generate_neighbor(const std::vector<int>& current, const Graph& g);
    
    /**
     * Check if given vertices form a valid clique
     * @param clique Candidate clique
     * @param g Input graph
     * @return true if valid clique
     */
    bool is_valid_clique(const std::vector<int>& clique, const Graph& g);
};

#endif // SIMULATED_ANNEALING_HPP
