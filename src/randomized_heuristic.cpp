#include "randomized_heuristic.hpp"
#include "greedy.hpp"

RandomizedHeuristic::RandomizedHeuristic(int num_restarts, int max_swaps, unsigned int seed)
    : num_restarts(num_restarts), max_swaps(max_swaps) {
    if (seed == 0) {
        std::random_device rd;
        rng.seed(rd());
    } else {
        rng.seed(seed);
    }
}

std::vector<int> RandomizedHeuristic::random_initial_clique(const Graph& g) {
    int n = g.num_vertices();
    std::vector<int> vertices;
    for (int v = 0; v < n; v++) {
        vertices.push_back(v);
    }
    
    // Shuffle vertices
    std::shuffle(vertices.begin(), vertices.end(), rng);
    
    // Greedily build clique from shuffled vertices
    std::vector<int> clique;
    for (int v : vertices) {
        bool can_add = true;
        for (int u : clique) {
            if (!g.has_edge(v, u)) {
                can_add = false;
                break;
            }
        }
        if (can_add) {
            clique.push_back(v);
        }
    }
    
    return clique;
}

std::vector<int> RandomizedHeuristic::local_search(const Graph& g, std::vector<int> current) {
    std::vector<int> best = current;
    int n = g.num_vertices();
    bool improved = true;
    int iterations = 0;
    
    while (improved && iterations < max_swaps) {
        improved = false;
        iterations++;
        
        std::unordered_set<int> current_set(current.begin(), current.end());
        
        // Try to add vertices that extend the clique
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
        
        // If we can directly extend, do it
        if (!candidates.empty()) {
            current.push_back(candidates[0]);
            improved = true;
            if (current.size() > best.size()) {
                best = current;
            }
            continue;
        }
        
        // Try swap operations: remove one, add one or more
        if (!current.empty()) {
            std::uniform_int_distribution<int> idx_dist(0, current.size() - 1);
            int remove_idx = idx_dist(rng);
            int removed = current[remove_idx];
            
            std::vector<int> temp = current;
            temp.erase(temp.begin() + remove_idx);
            std::unordered_set<int> temp_set(temp.begin(), temp.end());
            
            // Try to add vertices that weren't adjacent to removed vertex
            std::vector<int> swap_candidates;
            for (int v = 0; v < n; v++) {
                if (temp_set.find(v) == temp_set.end() && v != removed) {
                    bool can_add = true;
                    for (int u : temp) {
                        if (!g.has_edge(v, u)) {
                            can_add = false;
                            break;
                        }
                    }
                    if (can_add) {
                        swap_candidates.push_back(v);
                    }
                }
            }
            
            // Try adding multiple vertices
            std::vector<int> new_clique = temp;
            for (int v : swap_candidates) {
                bool can_add = true;
                for (int u : new_clique) {
                    if (!g.has_edge(v, u)) {
                        can_add = false;
                        break;
                    }
                }
                if (can_add) {
                    new_clique.push_back(v);
                }
            }
            
            if (new_clique.size() > current.size()) {
                current = new_clique;
                improved = true;
                if (current.size() > best.size()) {
                    best = current;
                }
            }
        }
    }
    
    return best;
}

std::vector<int> RandomizedHeuristic::find_clique(const Graph& g) {
    // Start with greedy solution
    std::vector<int> best = GreedyClique::find_clique(g);
    
    // Perform multiple random restarts
    for (int restart = 0; restart < num_restarts; restart++) {
        std::vector<int> initial;
        
        if (restart == 0) {
            // First restart uses greedy
            initial = best;
        } else {
            // Other restarts use random initialization
            initial = random_initial_clique(g);
        }
        
        // Perform local search
        std::vector<int> result = local_search(g, initial);
        
        // Update best if improved
        if (result.size() > best.size()) {
            best = result;
        }
    }
    
    return best;
}
