#include "cpu_optimized.hpp"
#include <stdexcept>

inline std::vector<int> CPUOptimized::bitset_to_vector(const std::bitset<MAX_VERTICES>& bs) {
    std::vector<int> result;
    for (int i = 0; i < n; i++) {
        if (bs[i]) {
            result.push_back(i);
        }
    }
    return result;
}

inline int CPUOptimized::choose_pivot(const std::bitset<MAX_VERTICES>& P,
                                      const std::bitset<MAX_VERTICES>& X) {
    int best_pivot = -1;
    int max_count = -1;
    
    // Check P
    for (int v = 0; v < n; v++) {
        if (P[v]) {
            int count = (P & neighbors[v]).count();
            if (count > max_count) {
                max_count = count;
                best_pivot = v;
            }
        }
    }
    
    // Check X
    for (int v = 0; v < n; v++) {
        if (X[v]) {
            int count = (P & neighbors[v]).count();
            if (count > max_count) {
                max_count = count;
                best_pivot = v;
            }
        }
    }
    
    return best_pivot;
}

inline void CPUOptimized::optimized_bk(std::bitset<MAX_VERTICES> R,
                                       std::bitset<MAX_VERTICES> P,
                                       std::bitset<MAX_VERTICES> X) {
    // OPTIMIZATION: Prune if current + remaining cannot beat best
    int current_size = R.count();
    int remaining_size = P.count();
    if (current_size + remaining_size <= (int)max_clique.size()) {
        return;  // Cannot find a larger clique in this branch
    }
    
    // Base case: P and X are empty
    if (P.none() && X.none()) {
        if (current_size > (int)max_clique.size()) {
            max_clique = bitset_to_vector(R);
        }
        return;
    }
    
    // Choose pivot
    int pivot = choose_pivot(P, X);
    
    // Compute candidates: P \ N(pivot)
    std::bitset<MAX_VERTICES> candidates;
    if (pivot != -1) {
        candidates = P & ~neighbors[pivot];
    } else {
        candidates = P;
    }
    
    // Iterate through candidates
    for (int v = 0; v < n; v++) {
        if (!candidates[v]) continue;
        
        // OPTIMIZATION: Check if this branch can improve best
        if (current_size + 1 + remaining_size <= (int)max_clique.size()) {
            break;  // No point continuing
        }
        
        // Create new sets using bitwise operations
        std::bitset<MAX_VERTICES> R_new = R;
        R_new.set(v);
        
        std::bitset<MAX_VERTICES> P_new = P & neighbors[v];
        std::bitset<MAX_VERTICES> X_new = X & neighbors[v];
        
        // Recurse
        optimized_bk(R_new, P_new, X_new);
        
        // Move v from P to X
        P.reset(v);
        X.set(v);
        remaining_size--;  // Update remaining size
    }
}

std::vector<int> CPUOptimized::find_maximum_clique(const Graph& g) {
    n = g.num_vertices();
    
    if (n > MAX_VERTICES) {
        throw std::runtime_error("Graph too large for CPU-optimized algorithm (max " + 
                               std::to_string(MAX_VERTICES) + " vertices)");
    }
    
    max_clique.clear();
    neighbors.clear();
    neighbors.resize(n);
    
    // Precompute neighbor bitsets
    for (int v = 0; v < n; v++) {
        const auto& vertex_neighbors = g.get_neighbors(v);
        for (int u : vertex_neighbors) {
            neighbors[v].set(u);
        }
    }
    
    // Initialize sets
    std::bitset<MAX_VERTICES> R;  // Empty
    std::bitset<MAX_VERTICES> P;  // All vertices
    std::bitset<MAX_VERTICES> X;  // Empty
    
    for (int v = 0; v < n; v++) {
        P.set(v);
    }
    
    // Run optimized algorithm
    optimized_bk(R, P, X);
    
    return max_clique;
}
