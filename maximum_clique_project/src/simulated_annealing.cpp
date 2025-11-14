#include "simulated_annealing.hpp"
#include "greedy.hpp"

SimulatedAnnealing::SimulatedAnnealing(double initial_temp, double cooling_rate, 
                                       int max_iterations, unsigned int seed)
    : initial_temperature(initial_temp), temperature(initial_temp),
      cooling_rate(cooling_rate), max_iterations(max_iterations) {
    if (seed == 0) {
        std::random_device rd;
        rng.seed(rd());
    } else {
        rng.seed(seed);
    }
}

bool SimulatedAnnealing::is_valid_clique(const std::vector<int>& clique, const Graph& g) {
    return g.is_clique(clique);
}

std::vector<int> SimulatedAnnealing::generate_neighbor(const std::vector<int>& current, 
                                                        const Graph& g) {
    std::vector<int> neighbor = current;
    std::uniform_int_distribution<int> op_dist(0, 2);
    int operation = op_dist(rng);
    
    int n = g.num_vertices();
    
    // Create set of current clique vertices for fast lookup
    std::unordered_set<int> current_set(current.begin(), current.end());
    
    if (operation == 0 && !neighbor.empty()) {
        // Remove a random vertex
        std::uniform_int_distribution<int> idx_dist(0, neighbor.size() - 1);
        int remove_idx = idx_dist(rng);
        neighbor.erase(neighbor.begin() + remove_idx);
    } 
    else if (operation == 1) {
        // Try to add a vertex that's connected to all current vertices
        std::vector<int> candidates;
        for (int v = 0; v < n; v++) {
            if (current_set.find(v) == current_set.end()) {
                bool can_add = true;
                for (int u : current) {
                    if (!g.has_edge(v, u)) {
                        can_add = false;
                        break;
                    }
                }
                if (can_add) {
                    candidates.push_back(v);
                }
            }
        }
        
        if (!candidates.empty()) {
            std::uniform_int_distribution<int> cand_dist(0, candidates.size() - 1);
            neighbor.push_back(candidates[cand_dist(rng)]);
        }
    } 
    else if (operation == 2 && !neighbor.empty()) {
        // Swap: remove one vertex and add another
        std::uniform_int_distribution<int> idx_dist(0, neighbor.size() - 1);
        int remove_idx = idx_dist(rng);
        int removed_vertex = neighbor[remove_idx];
        neighbor.erase(neighbor.begin() + remove_idx);
        current_set.erase(removed_vertex);
        
        // Try to add a vertex
        std::vector<int> candidates;
        for (int v = 0; v < n; v++) {
            if (current_set.find(v) == current_set.end()) {
                bool can_add = true;
                for (int u : neighbor) {
                    if (!g.has_edge(v, u)) {
                        can_add = false;
                        break;
                    }
                }
                if (can_add) {
                    candidates.push_back(v);
                }
            }
        }
        
        if (!candidates.empty()) {
            std::uniform_int_distribution<int> cand_dist(0, candidates.size() - 1);
            neighbor.push_back(candidates[cand_dist(rng)]);
        }
    }
    
    return neighbor;
}

std::vector<int> SimulatedAnnealing::find_clique(const Graph& g) {
    // Start with greedy solution
    std::vector<int> current = GreedyClique::find_clique(g);
    std::vector<int> best = current;
    
    temperature = initial_temperature;
    std::uniform_real_distribution<double> prob_dist(0.0, 1.0);
    
    for (int iter = 0; iter < max_iterations; iter++) {
        // Generate neighbor solution
        std::vector<int> neighbor = generate_neighbor(current, g);
        
        // Validate neighbor is a clique
        if (!is_valid_clique(neighbor, g)) {
            continue;
        }
        
        // Calculate energy difference (negative because we want to maximize)
        int delta_E = current.size() - neighbor.size();
        
        // Accept better solutions always, worse solutions with probability
        bool accept = false;
        if (delta_E < 0) {
            // Better solution (larger clique)
            accept = true;
        } else if (delta_E > 0 && temperature > 0) {
            // Worse solution - accept with probability
            double acceptance_prob = std::exp(-delta_E / temperature);
            accept = (prob_dist(rng) < acceptance_prob);
        } else {
            // Same size
            accept = (prob_dist(rng) < 0.5);
        }
        
        if (accept) {
            current = neighbor;
            
            // Update best solution
            if (current.size() > best.size()) {
                best = current;
            }
        }
        
        // Cool down
        temperature *= cooling_rate;
    }
    
    return best;
}
